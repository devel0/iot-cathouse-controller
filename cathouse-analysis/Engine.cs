using System;
using System.Globalization;
using System.IO;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using static System.FormattableString;
using System.Collections.Generic;

namespace cathouse_analysis
{

    public class PortInfo
    {

        HttpClient client;

        public int PortNumber { get; private set; }

        /// <summary>
        /// value 0..1 of preference for this port to be on
        /// </summary>
        public double Preference { get; private set; }

        public PortInfo(HttpClient client, int port, double pref = 1d)
        {
            this.client = client;
            PortNumber = port;
        }

        bool initialized = false;

        /// <summary>
        /// use Read() to update with current real port status
        /// </summary>
        public bool IsOn { get; private set; }

        /// <summary>
        /// seconds from which port is on from last turn off (updated by Read())
        /// </summary>        
        public double SecOn { get; private set; }

        /// <summary>
        /// seconds from which port is on since program start (updated by Read())
        /// </summary>
        public double TotalSecOn { get; private set; }

        public const double POWER_W = 23d;

        /// <summary>
        /// total power consumption
        /// </summary>
        public double Wh
        {
            get
            {
                return POWER_W * TotalSecOn / 60d / 60d;
            }
        }

        /// <summary>
        /// mean power
        /// </summary>
        public double W(double runtimeHour)
        {
            return Wh / runtimeHour;
        }

        DateTime? lastRead = null;

        /// <summary>
        /// also updates IsOn
        /// </summary>
        public async Task<bool> Read()
        {
            var timediff = TimeSpan.FromSeconds(0);

            var qnow = await client.GetStringAsync($"http://cathouse.searchathing.com/port/get/{PortNumber}") == "1";            
            if (qnow)
            {
                if (IsOn && lastRead.HasValue)
                {
                    var t = (DateTime.Now - lastRead.Value).TotalSeconds;
                    SecOn += t;
                    TotalSecOn += t;
                }
            }
            lastRead = DateTime.Now;

            return IsOn = qnow;
        }

        public async Task Write(bool on)
        {
            if (initialized)
            {
                if (IsOn && on) return;
                if (!IsOn && !on) return;
            }
            initialized = true;

            System.Console.Write($"port {PortNumber} {(on ? "ON" : "OFF")} => ");
            var res = await client.GetStringAsync($"http://cathouse.searchathing.com/port/set/{PortNumber}/{(on ? "1" : "0")}");
            System.Console.WriteLine(res);
            if (!on) SecOn = 0;
        }
    }

    public class Engine
    {
        HttpClient client = new HttpClient();

        public Engine()
        {
        }

        #region get temperature helpers
        async Task<double> GetTBottom()
        {
            var s = await client.GetStringAsync($"http://cathouse.searchathing.com/temp/28b03724070000c8");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }

        async Task<double> GetTAmbient()
        {
            var s = await client.GetStringAsync($"http://cathouse.searchathing.com/temp/28f00a3b05000038");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }

        async Task<double> GetTWood()
        {
            var s = await client.GetStringAsync($"http://cathouse.searchathing.com/temp/28e2cc23070000d8");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }

        async Task<double> GetTExtern()
        {
            var s = await client.GetStringAsync($"http://cathouse.searchathing.com/temp/28d12b5b0500001c");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }
        #endregion

        public async Task Run()
        {
            System.Console.WriteLine("RUN");
            var p1 = new PortInfo(client, 1, 1.0);
            var p2 = new PortInfo(client, 2, 1.0);
            var p3 = new PortInfo(client, 3, 1.0);
            var p4 = new PortInfo(client, 4, 1.0);
            var ports = new List<PortInfo>() { p1, p2, p3, p4 };
            foreach (var x in ports)
            {
                System.Console.WriteLine($"Init port {x.PortNumber} to OFF");
                await x.Write(false);
            }

            System.Console.WriteLine("ports setup done");

            var dtBegin = DateTime.Now;

            try
            {
                var sw = new StreamWriter("/home/devel0/devel-tmp/cathouse-lab/data2.csv", true);

                var dt = DateTime.Now;
                DateTime? dtCooldownStarted = null;
                var cooldownTime = TimeSpan.FromMinutes(2);
                var portSwitchingTime = TimeSpan.FromSeconds(2.5 * 60);

                while (true)
                {
                    try
                    {
                        var tbottom = await GetTBottom();
                        var tambient = await GetTAmbient();
                        var twood = await GetTWood();
                        var textern = await GetTExtern();

                        var dtstr = DateTime.Now.ToString("O");
                        var catinthere = false;

                        foreach (var p in ports) await p.Read();

                        var runtimeHr = (DateTime.Now - dtBegin).TotalHours;

                        var str =
                            Invariant($"{dtstr} {textern} {tambient} {tbottom} {twood}") +
                            Invariant($" {(p1.IsOn ? "1" : "0")} {p1.SecOn} {p1.TotalSecOn} {p1.Wh} {p1.W(runtimeHr)}") +
                            Invariant($" {(p2.IsOn ? "1" : "0")} {p2.SecOn} {p2.TotalSecOn} {p2.Wh} {p2.W(runtimeHr)}") +
                            Invariant($" {(p3.IsOn ? "1" : "0")} {p3.SecOn} {p3.TotalSecOn} {p3.Wh} {p3.W(runtimeHr)}") +
                            Invariant($" {(p4.IsOn ? "1" : "0")} {p4.SecOn} {p4.TotalSecOn} {p4.Wh} {p4.W(runtimeHr)}");

                        System.Console.WriteLine(str);
                        System.Console.WriteLine(Invariant($"Total Wh={ports.Sum(p => p.Wh)} W={ports.Sum(p => p.W(runtimeHr))}"));
                        System.Console.WriteLine();

                        /*System.Console.WriteLine(string.Format("{0} bottom:{1,-8} ambient:{2,-8} wood:{3,-8} extern:{4,-8} power(wH):{5,-8:0.00} pmean(w):{6,-8:0.00} runtime(hr):{7,-8:0.000}",
                        dtstr, tbottom, tambient, twood, textern,
                        wH,
                        runtime >= 1.0 ? wH / runtime : wH,
                        runtime));*/

                        if ((DateTime.Now - dt).TotalSeconds >= 60) // flush data recording
                        {
                            /* sw.WriteLine(str);
                            sw.Flush();*/
                            dt = DateTime.Now;
                        }

                        if (!dtCooldownStarted.HasValue ||
                            (dtCooldownStarted.HasValue && (DateTime.Now - dtCooldownStarted.Value >= cooldownTime)))
                        {
                            dtCooldownStarted = null;

                            // if temp exceed disable all ports and enter cooldown mode
                            if (tambient >= 18 || tbottom >= 30 || twood >= 40)
                            {
                                dtCooldownStarted = DateTime.Now;
                                foreach (var x in ports) await x.Write(false);
                            }
                            // if temp near to exceed enable only port with max prefernce
                            else if (tbottom >= 28 || twood >= 38)
                            {
                                var plst = ports.OrderByDescending(w => w.Preference).ToList();
                                for (int i = 0; i < plst.Count; ++i)
                                {
                                    await plst[i].Write(i == 0 ? true : false);
                                }
                            }
                            else
                            {
                                if (tambient < 10) // enable all ports to get ambient over 10C
                                {
                                    foreach (var x in ports) await x.Write(true);
                                }
                                else
                                {
                                    // if current enable port exceeded port runtime
                                    var q = ports.FirstOrDefault(w => w.SecOn > portSwitchingTime.TotalSeconds);
                                    if (q != null)
                                    {
                                        var qidx = (ports.IndexOf(q) + 1) % ports.Count;
                                        await q.Write(false); // disable current port
                                        await ports[qidx].Write(true); // enable next
                                    }
                                    else if (ports.All(w => !w.IsOn)) // if all ports off then enable port with max pref
                                    {
                                        var plst = ports.OrderByDescending(w => w.Preference).ToList();
                                        for (int i = 0; i < plst.Count; ++i)
                                        {
                                            await plst[i].Write(i == 0 ? true : false);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        System.Console.WriteLine($"error: {ex.Message}");
                    }

                    await Task.Delay(5 * 1000);
                }
            }
            catch (Exception ex)
            {
                System.Console.WriteLine($"error: {ex.Message}");
                foreach (var p in ports) p.Write(false);
            }
        }
    }

}
