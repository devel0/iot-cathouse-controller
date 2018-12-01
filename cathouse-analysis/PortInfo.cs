
using System;
using System.Net.Http;
using System.Threading.Tasks;

namespace cathouse_analysis
{
    public class PortInfo
    {

        /// <summary>
        /// measured 107W with all 4 ports enabled
        /// </summary>
        public const double POWER_W = 107d / 4;

        /// <summary>
        /// min time for which a port still off to avoid bouncing
        /// </summary>
        public TimeSpan COOLDOWN_TIMESPAN { get; private set; } = TimeSpan.FromSeconds(60);
        
        public TimeSpan MINONTIME { get; private set; } = TimeSpan.FromSeconds(60);

        HttpClient client;

        public int PortNumber { get; private set; }

        /// <summary>
        /// preference value to use this port in heating system
        /// </summary>
        public double Preference { get; private set; }

        public PortInfo(HttpClient client, int port, double preference = 1.0)
        {
            this.client = client;
            PortNumber = port;
            this.Preference = preference;

            InitTime = DateTime.Now;
            Task.Run(async () =>
            {
                await SafeWrite(false);
            }).Wait();
            IsOn = false;
        }

        /// <summary>
        /// datetime of port initialization
        /// </summary>        
        public DateTime InitTime { get; private set; }

        /// <summary>
        /// true if port is on
        /// </summary>
        public bool IsOn { get; private set; }

        /// <summary>
        /// timestamp for off/on state change
        /// </summary>
        DateTime OffOnTimestamp;

        /// <summary>
        /// timespan from which port is on from last off-on switch
        /// </summary>        
        public TimeSpan OnTime
        {
            get
            {
                if (!IsOn) return TimeSpan.FromSeconds(0);
                return DateTime.Now - OffOnTimestamp;
            }
        }

        TimeSpan OnTimeTotalBacklog = TimeSpan.FromSeconds(0);

        /// <summary>
        /// total timespan for port on state
        /// </summary>        
        public TimeSpan OnTimeTotal
        {
            get
            {
                return OnTimeTotalBacklog + OnTime;
            }
        }

        /// <summary>
        /// timestamp for on/off state change
        /// </summary>
        DateTime OnOffTimestamp;

        /// <summary>
        /// timespan from which port is off from last on-off switch
        /// </summary>        
        public TimeSpan OffTime
        {
            get
            {
                if (IsOn) return TimeSpan.FromSeconds(0);
                return DateTime.Now - OnOffTimestamp;
            }
        }

        /// <summary>
        /// total running time ( either on/off )
        /// </summary>
        public TimeSpan Runtime
        {
            get
            {
                return DateTime.Now - InitTime;
            }
        }

        /// <summary>
        /// total power consumption
        /// </summary>
        public double Wh
        {
            get
            {
                return POWER_W * OnTimeTotal.TotalHours;
            }
        }

        /// <summary>
        /// mean power
        /// </summary>
        public double W
        {
            get
            {
                return Wh / Runtime.TotalHours;
            }
        }

        /// <summary>
        /// verifies if IsOn keep in sync with current real port value and adjust in case
        /// </summary>
        public async Task Verify()
        {
            var IsOnBk = IsOn;
            await SafeRead();
            if (IsOnBk != IsOn)
            {
                System.Console.WriteLine($"port {PortNumber} status adjusted to real state [on={IsOn}]");
            }
        }

        /// <summary>
        /// turn on/off port
        /// it consider cooldown time if it was on-off
        /// </summary>
        public async Task Write(bool on, bool force = false)
        {
            // it was on and want to off it ( start cooldown mode )
            if ((IsOn || force) && !on)
            {
                if (OnTime > MINONTIME || force)
                {
                    OnTimeTotalBacklog += OnTime;
                    await SafeWrite(false);                
                    OnOffTimestamp = DateTime.Now;
                    IsOn = false;
                }
                else
                    System.Console.WriteLine($"can't disable port {PortNumber} in min on time mode");
            }
            // enable port ( if not in cooldown mode )
            else if ((!IsOn || force) && on)
            {
                if (OffTime > COOLDOWN_TIMESPAN || force)
                {
                    await SafeWrite(true);
                    OffOnTimestamp = DateTime.Now;
                    IsOn = true;
                }
                else
                    System.Console.WriteLine($"can't enable port {PortNumber} in cooldown mode");
            }
        }

        async Task SafeRead()
        {
            while (true)
            {
                try
                {
                    IsOn = await client.GetStringAsync($"http://cathouse.searchathing.com/port/get/{PortNumber}") == "1";
                    break;
                }
                catch (Exception ex)
                {
                    System.Console.WriteLine($"error [{ex.Message}] reading port {PortNumber} status...retrying");
                    await Task.Delay(1000);
                }
            }
        }

        async Task SafeWrite(bool on)
        {
            while (true)
            {
                try
                {
                    var res = await client.GetStringAsync($"http://cathouse/port/set/{PortNumber}/{(on ? "1" : "0")}");
                    if (res == "OK")
                    {
                        IsOn = on;
                        System.Console.WriteLine($"TURN PORT {PortNumber} {(IsOn ? "ON" : "OFF")}");
                        break;
                    }
                }
                catch (Exception ex)
                {
                    System.Console.WriteLine($"error [{ex.Message}] writing port {PortNumber}...retrying");
                    await Task.Delay(1000);
                }
            }
        }

    }

}
