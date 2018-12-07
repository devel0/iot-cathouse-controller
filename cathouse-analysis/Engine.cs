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
using static System.Math;

namespace cathouse_analysis
{

    public class Engine
    {
        HttpClient client = new HttpClient();

        public const double TBOTTOM_LIMIT = 40d;

        public const double TWOOD_LIMIT = 50d;

        public const double TAMBIENT_LIMIT = 17d;

        // 14d -> 70W

        public const double TAMBIENT_VS_EXTERN_GTE_SYSOFF = 11d;

        public const double TAMBIENT_VS_EXTERN_LTE_SYSON = 10d;

        public const double TBOTTOM_GTE_FANON = 30d;

        public const double TBOTTOM_LTE_FANOFF = 25d;

        public const double AUTOACTIVATE_WOOD_BOTTOM_DELTA = 3d;

        //public const double AUTOACTIVATE_

        public int AUTODEACTIVATE_EXCURSION_SAMPLE_COUNT = 5;

        /// <summary>
        /// record AUTODEACTIVATE_MEAN_SAMPLE_COUNT in AUTODEACTIVATE_MEAN_SAMPLE_TOTAL_TIMESPAN given timespan
        /// for example in 50 min reocord 5 values of mean temperature to estimate mean temperature in 50 min
        /// </summary>        
        public TimeSpan AUTODEACTIVATE_EXCURSION_SAMPLE_TOTAL_TIMESPAN = TimeSpan.FromMinutes(30);

        public double AUTODEACTIVATE_WOOD_DELTA_LT = 8.5d;

        /// <summary>
        /// minimum timespan from deactivation of system to allow wood tend to bottom temperature
        /// thus inhibit autoactivate during this phase
        /// </summary>
        public TimeSpan AUTODEACTIVATE_INHIBIT_AUTOACTIVATE_MINTIMESPAN = TimeSpan.FromMinutes(30);

        /// <summary>
        /// extern temperature limit for system to be enabled
        /// </summary>
        public const double TEXTERN_GTE_SYSOFF = 14d;

        public bool SystemIsOn = false;

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

        async Task TurnOffSystem()
        {
            foreach (var p in ports) await p.Write(false);
            await fan.Write(false);
            SystemIsOn = false;
        }

        PortInfo fan = null;
        List<PortInfo> ports = null;

        public async Task Run()
        {
            System.Console.WriteLine("RUN");
            var p2 = new PortInfo(client, 2, preference: 1.0);
            var p4 = new PortInfo(client, 4, preference: .9);
            var p1 = new PortInfo(client, 1, preference: .8);
            var p3 = new PortInfo(client, 3, preference: .7);

            fan = new PortInfo(client, 6);

            //var targetTempMaxDistanceActivators = new[] { 0, 1, 2, 3 };
            ports = new List<PortInfo>() { p1, p2, p3, p4 };

            await TurnOffSystem();

            System.Console.WriteLine("ports setup done");

            var excursionSamples = new List<ExcursionSample>();

            var dtBegin = DateTime.Now;

#if !DEBUG
            try
#endif
            {
                var sw = new StreamWriter("/home/devel0/devel-tmp/cathouse-lab/data2.csv", true);

                var dt = DateTime.Now;
                DateTime? dtCooldownStarted = null;
                // cooldown time if temp exceed max values
                var COOLDOWN_TIME = TimeSpan.FromMinutes(2);

                DateTime? dtDeactivateStarted = null;

                while (true)
                {
#if !DEBUG
                    try
#endif
                    {
                        var tbottom = await GetTBottom();
                        var tambient = await GetTAmbient();
                        var twood = await GetTWood();
                        var textern = await GetTExtern();

                        #region fan control
                        {
                            // control fan based on bottom temp
                            if (tbottom >= TBOTTOM_GTE_FANON)
                                await fan.Write(true);
                            else if (tbottom <= TBOTTOM_LTE_FANOFF)
                                await fan.Write(false);
                        }
                        #endregion

                        #region logdata
                        {
                            var dtstr = DateTime.Now.ToString("O");
                            //var catinthere = false;

                            foreach (var p in ports) await p.Verify();

                            var deltawood = 0d;
                            if (excursionSamples.Count > 0 && excursionSamples.First().Count > 0)
                            {
                                var min = Min(excursionSamples.Min(w => w.Min), twood);
                                var max = Max(excursionSamples.Max(w => w.Max), twood);
                                deltawood = max - min;
                            }

                            var str = Invariant($"{dtstr} {(SystemIsOn ? "1" : "0")} {deltawood} {(fan.IsOn ? "1" : "0")} {((p1.IsOn || p2.IsOn || p3.IsOn || p4.IsOn) ? "1" : "0")} {textern} {tambient} {tbottom} {twood}");

                            System.Console.WriteLine();
                            System.Console.WriteLine("date syson deltawood fan heating text tamb tbott twood");
                            System.Console.WriteLine(str);
                            System.Console.WriteLine();

                            //    if ((DateTime.Now - dt).TotalSeconds >= 60) // flush data recording
                            {
                                sw.WriteLine(str);
                                sw.Flush();
                                dt = DateTime.Now;
                            }
                        }
                        #endregion

                        #region check system reactivation
                        var needActivate = false;
                        System.Console.WriteLine($"  Abs(twood - tbottom) = {Abs(twood - tbottom)} >= AUTOACTIVATE_WOOD_BOTTOM_DELTA = {AUTOACTIVATE_WOOD_BOTTOM_DELTA} : {Abs(twood - tbottom) >= AUTOACTIVATE_WOOD_BOTTOM_DELTA}");
                        if (!SystemIsOn && (textern < TEXTERN_GTE_SYSOFF || textern - 1 > tambient))
                        {
                            //  check if cat in there to activate if any ( but must not in autodeactivate min timespan )
                            if (((Abs(twood - tbottom) >= AUTOACTIVATE_WOOD_BOTTOM_DELTA) &&
                                (!dtDeactivateStarted.HasValue
                                ||
                                (DateTime.Now - dtDeactivateStarted.Value) > AUTODEACTIVATE_INHIBIT_AUTOACTIVATE_MINTIMESPAN)))
                            {
                                needActivate = true;
                                System.Console.WriteLine($"---> NEED ACTIVATE");
                                System.Console.WriteLine($"  textern < {TEXTERN_GTE_SYSOFF} : {textern < TEXTERN_GTE_SYSOFF}");
                                System.Console.WriteLine($"  textern-1 > tambient : {textern - 1 > tambient}");
                                System.Console.WriteLine($"  dtDeactivateStarted.HasValue = {dtDeactivateStarted.HasValue}");
                                if (dtDeactivateStarted.HasValue)
                                    System.Console.WriteLine($"  (Datetime.Now - dtDeactivateStarted.Value) > AUTODEACTIVATE_INHIBIT_AUTOACTIVATE_MINTIMESPAN) : {(DateTime.Now - dtDeactivateStarted.Value) > AUTODEACTIVATE_INHIBIT_AUTOACTIVATE_MINTIMESPAN}");
                            }
                        }
                        #endregion

                        #region SystemOn ( or needActivation )
                        if (SystemIsOn || needActivate)
                        {
                            dtDeactivateStarted = null;

                            if (!SystemIsOn)
                            {
                                System.Console.WriteLine($"===> TURN SYSTEM ON");
                                SystemIsOn = true;
                            }

                            #region eval deactivate based on excursion values
                            {
                                ExcursionSample es = null;
                                var excursionSamplesMaxCount = AUTODEACTIVATE_EXCURSION_SAMPLE_COUNT;
                                var excursionSampleMaxAge = AUTODEACTIVATE_EXCURSION_SAMPLE_TOTAL_TIMESPAN / excursionSamplesMaxCount;

                                if (excursionSamples.Count == 0)
                                {
                                    es = new ExcursionSample(excursionSampleMaxAge);
                                    excursionSamples.Add(es);
                                }
                                else
                                {
                                    es = excursionSamples.Last();
                                    // if this sample set expired create new one and if count exceed excursionSamplesMaxCount
                                    // - eval delta to understand if cat in there
                                    // - remove oldest                            
                                    if (!es.Add(twood))
                                    {
                                        es = new ExcursionSample(excursionSampleMaxAge);
                                        es.Add(twood);

                                        if (excursionSamples.Count > excursionSamplesMaxCount)
                                        {
                                            var min = Min(excursionSamples.Min(w => w.Min), twood);
                                            var max = Max(excursionSamples.Max(w => w.Max), twood);
                                            var delta = max - min;

                                            if (delta < AUTODEACTIVATE_WOOD_DELTA_LT)
                                            {
                                                dtDeactivateStarted = DateTime.Now;
                                            }

                                            System.Console.WriteLine($"________EVAL AUTODEACTIVATE = {dtDeactivateStarted.HasValue} ; MIN = {min} ; MAX = {max} ; DELTA = {delta}");

                                            var first = excursionSamples.First();
                                            excursionSamples.Remove(first);


                                            if (dtDeactivateStarted.HasValue)
                                            {
                                                System.Console.WriteLine($"===> TURN SYSTEM OFF");

                                                await TurnOffSystem();
                                                continue;
                                            }
                                        }

                                        excursionSamples.Add(es);
                                    }

                                    {
                                        var min = Min(excursionSamples.Min(w => w.Min), twood);
                                        var max = Max(excursionSamples.Max(w => w.Max), twood);
                                        var delta = max - min;
                                        System.Console.WriteLine($"es slots cnt={excursionSamples.Count} esidx={excursionSamples.IndexOf(es)} : last excursion sample count={es.Count} age={es.Age} maxAge={es.MaxAge} min={es.Min} max={es.Max} delta={es.Max - es.Min} oldest sample timestamp = {es.OldestSampleTimestamp}");
                                        System.Console.WriteLine($"all excursion slots min={min} max={max} delta={delta}");
                                    }
                                }
                            }
                            #endregion

                            var tambient_gte_sysoff = textern + TAMBIENT_VS_EXTERN_GTE_SYSOFF;
                            var tambient_lte_syson = textern + TAMBIENT_VS_EXTERN_LTE_SYSON;

                            if (textern > TEXTERN_GTE_SYSOFF && tambient + 1 >= textern) // disable system if extern temp hot and extern-1 > ambient
                            {
                                foreach (var p in ports) await p.Write(false);
                                await fan.Write(false);

                                System.Console.WriteLine($"===> TURN SYSTEM OFF because textern = {textern} and tambient+1 >= textern: {tambient}+1 >= {textern}");
                                await TurnOffSystem();
                                continue;
                            }
                            else if (!dtCooldownStarted.HasValue ||
                                 (dtCooldownStarted.HasValue && (DateTime.Now - dtCooldownStarted.Value >= COOLDOWN_TIME)))
                            {
                                dtCooldownStarted = null;

                                // if temp exceed disable all ports and enter cooldown mode
                                if (tbottom >= TBOTTOM_LIMIT || twood >= TWOOD_LIMIT || tambient >= TAMBIENT_LIMIT)
                                {
                                    dtCooldownStarted = DateTime.Now;
                                    foreach (var x in ports) await x.Write(false, force: true);
                                }
                                // run ports to increase tambient
                                else
                                {
                                    if (tambient <= tambient_lte_syson)
                                    {
                                        foreach (var p in ports) await p.Write(true);
                                    }
                                    else if (tambient >= tambient_gte_sysoff)
                                    {
                                        foreach (var p in ports) await p.Write(false);
                                    }

                                    System.Console.WriteLine($"FAN={fan.IsOn}");
                                }
                            }
                        }
                        #endregion
                    }
#if !DEBUG
                    catch (Exception ex)
            {
                System.Console.WriteLine($"*** error: {ex.Message}");
            }
#endif

                    await Task.Delay(5 * 1000);
                }
            }
#if !DEBUG
            catch (Exception ex)
            {
                System.Console.WriteLine($"*** error: {ex.Message}");
                foreach (var p in ports) await p.Write(false, force: true);
            }
#endif
        }
    }

}
