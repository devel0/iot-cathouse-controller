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

        public const double TAMBIENT_VS_EXTERN_GTE_SYSOFF = 12d;

        public const double TAMBIENT_VS_EXTERN_LTE_SYSON = 4d;

        /*        public const double TBOTTOM_GTE_FANON = 35d;

                public const double TBOTTOM_LTE_FANOFF = 25d;*/

        public const double AUTOACTIVATE_WOOD_BOTTOM_DELTA = 2d;

        //public const double AUTOACTIVATE_

        public int AUTODEACTIVATE_EXCURSION_SAMPLE_COUNT = 5;

        /// <summary>
        /// record AUTODEACTIVATE_MEAN_SAMPLE_COUNT in AUTODEACTIVATE_MEAN_SAMPLE_TOTAL_TIMESPAN given timespan
        /// for example in 50 min reocord 5 values of mean temperature to estimate mean temperature in 50 min
        /// </summary>        
        public TimeSpan AUTODEACTIVATE_EXCURSION_SAMPLE_TOTAL_TIMESPAN = TimeSpan.FromMinutes(90);

        public double AUTODEACTIVATE_BOTTOM_DELTA_LT = 8.5d;

        /// <summary>
        /// minimum timespan from deactivation of system to allow wood tend to bottom temperature
        /// thus inhibit autoactivate during this phase
        /// </summary>
        public TimeSpan AUTODEACTIVATE_INHIBIT_AUTOACTIVATE_MINTIMESPAN = TimeSpan.FromMinutes(30);

        /// <summary>
        /// extern temperature limit for system to be enabled
        /// </summary>
        public const double TEXTERN_GTE_SYSOFF = 14d;

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

        async Task<double> GetWeightADC()
        {
            var s = await client.GetStringAsync($"http://cathouse.searchathing.com/port/get/7");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }
        #endregion

        async Task TurnOffSystem()
        {
            foreach (var p in ports) await p.Write(false);
            await fan.Write(false);
        }

        PortInfo fan = null;
        List<PortInfo> ports = null;

        enum HeatCycleType { none, full, standby };

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

            var currentHeatCycle = HeatCycleType.none;
            var heatCycleSpan = TimeSpan.FromSeconds(0);
            var dtHeatCycleBegin = DateTime.Now;

#if !DEBUG
            try
#endif
            {
                var sw = new StreamWriter("/home/devel0/devel-tmp/cathouse-lab/data2.csv", true);

                var dt = DateTime.Now;

                // cooldown time if temp exceed max values
                var COOLDOWN_TIME = TimeSpan.FromMinutes(2);
                var wlst = new List<double>();

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
                        var weightadc = await GetWeightADC();

                        if (wlst.Count > 5) wlst.RemoveAt(0);
                        wlst.Add(weightadc);

                        var wmean = (wlst.Sum(w => w) / wlst.Count);
                        System.Console.WriteLine($"W={weightadc} [mean={wmean}]");
                        var catisinthere = wmean >= 120;

                        if (!catisinthere)
                        {
                            foreach (var p in ports)
                            {
                                await p.Write(false);
                            }
                            await fan.Write(false);
/* 
                            if (currentHeatCycle != HeatCycleType.none)
                            {
                                System.Console.WriteLine($"<===== {DateTime.Now} cat exited");
                                currentHeatCycle = HeatCycleType.none;
                            }*/
                        }
                        else
                        {
                            switch (currentHeatCycle)
                            {
                                case HeatCycleType.none:
                                    {
                                        dtHeatCycleBegin = DateTime.Now;
                                        currentHeatCycle = HeatCycleType.full;
                                        System.Console.WriteLine($"=====> {DateTime.Now} cat entered");

                                        foreach (var p in ports)
                                        {
                                            await p.Write(true);
                                        }
                                        await fan.Write(true);
                                    }
                                    break;
                            }

                            switch (currentHeatCycle)
                            {
                                case HeatCycleType.full:
                                    {
                                        foreach (var p in ports)
                                        {
                                            await p.Write(true);
                                        }
                                        await fan.Write(true);

                                        if ((DateTime.Now - dtHeatCycleBegin) > TimeSpan.FromMinutes(20))
                                        {
                                            currentHeatCycle = HeatCycleType.standby;
                                            dtHeatCycleBegin = DateTime.Now;

                                            foreach (var p in ports)
                                            {
                                                if (p != p2)
                                                    await p.Write(false);
                                                else
                                                    await p.Write(true);
                                            }
                                            await fan.Write(false);
                                        }
                                    }
                                    break;

                                case HeatCycleType.standby:
                                    {
                                        foreach (var p in ports)
                                        {
                                            if (p != p2)
                                                await p.Write(false);
                                            else
                                                await p.Write(true);
                                        }
                                        await fan.Write(false);

                                        if ((DateTime.Now - dtHeatCycleBegin) > TimeSpan.FromMinutes(30))
                                        {
                                            currentHeatCycle = HeatCycleType.full;
                                            dtHeatCycleBegin = DateTime.Now;

                                            foreach (var p in ports)
                                            {
                                                await p.Write(true);
                                            }
                                            await fan.Write(true);
                                        }
                                    }
                                    break;
                            }
                        }

                        await Task.Delay(5000);
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
