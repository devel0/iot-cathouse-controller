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

        public const double TAMBIENT_VS_EXTERN_TARGET = 10.0d;

        /// <summary>
        /// min extern temperature to enable at least 1 port
        /// </summary>
        public const double TEXTERN_MIN = 10d;

        //public const double TBOTTOM_TARGET = 20d;

        public const double W_TARGET = 33d;

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
            var p2 = new PortInfo(client, 2, preference: 1.0);            
            var p4 = new PortInfo(client, 4, preference: .9);
            var p1 = new PortInfo(client, 1, preference: .8);
            var p3 = new PortInfo(client, 3, preference: .7);
            var targetTempMaxDistanceActivators = new[] { 0, 1, 2, 3 };
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
                // cooldown time if temp exceed max values
                var COOLDOWN_TIME = TimeSpan.FromMinutes(2);

                while (true)
                {
                    try
                    {
                        var tbottom = await GetTBottom();
                        var tambient = await GetTAmbient();
                        var twood = await GetTWood();
                        var textern = await GetTExtern();

                        var tambient_target = textern + TAMBIENT_VS_EXTERN_TARGET;

                        var dtstr = DateTime.Now.ToString("O");
                        var catinthere = false;

                        foreach (var p in ports) await p.Verify();

                        var runtimeHr = (DateTime.Now - dtBegin).TotalHours;

                        var str =
                            Invariant($"{dtstr} {textern} {tambient} {tbottom} {twood}") +
                            Invariant($" {(p1.IsOn ? "1" : "0")} {p1.OnTime.TotalSeconds} {p1.OnTimeTotal.TotalSeconds} {p1.Wh} {p1.W}") +
                            Invariant($" {(p2.IsOn ? "1" : "0")} {p2.OnTime.TotalSeconds} {p2.OnTimeTotal.TotalSeconds} {p2.Wh} {p2.W}") +
                            Invariant($" {(p3.IsOn ? "1" : "0")} {p3.OnTime.TotalSeconds} {p3.OnTimeTotal.TotalSeconds} {p3.Wh} {p3.W}") +
                            Invariant($" {(p4.IsOn ? "1" : "0")} {p4.OnTime.TotalSeconds} {p4.OnTimeTotal.TotalSeconds} {p4.Wh} {p4.W}");

                        var W = ports.Sum(p => p.W);
                        var Wh = ports.Sum(p => p.Wh);

                        System.Console.WriteLine(str);
                        System.Console.WriteLine(Invariant($"Total Wh={Wh} W={W}"));
                        System.Console.WriteLine();

                        //    if ((DateTime.Now - dt).TotalSeconds >= 60) // flush data recording
                        {
                            sw.WriteLine(str);
                            sw.Flush();
                            dt = DateTime.Now;
                        }

                        if (!dtCooldownStarted.HasValue ||
                            (dtCooldownStarted.HasValue && (DateTime.Now - dtCooldownStarted.Value >= COOLDOWN_TIME)))
                        {
                            dtCooldownStarted = null;

                            // if temp exceed disable all ports and enter cooldown mode
                            if (tbottom >= TBOTTOM_LIMIT || twood >= TWOOD_LIMIT || (tambient >= tambient_target && textern > TEXTERN_MIN))
                            {
                                dtCooldownStarted = DateTime.Now;
                                foreach (var x in ports) await x.Write(false, force: true);
                            }
                            // run ports to increase tambient
                            else
                            {
                                var tdifftgt = "ambient";
                                var tdiff = tambient_target - tambient;

                                {
                                    var ordered_ports = ports.OrderByDescending(w => w.Preference).ToList();
                                    var boosted = false;
                                    for (int i = 0; i < ordered_ports.Count; ++i)
                                    {
                                        var p = ordered_ports[i];

                                        var state = tdiff >= targetTempMaxDistanceActivators[i];

                                        if (!boosted && !state && W < W_TARGET && tdiff > .05d)
                                        {
                                            state = true;
                                            boosted = true;
                                        }

                                        if (textern <= TEXTERN_MIN && i == 0)
                                            state = true;

                                        await ordered_ports[i].Write(state);
                                    }
                                }

                                System.Console.WriteLine($"tdiff [{tdifftgt}] from target = {tdiff}C   tambtgt = {tambient_target}C    enabled ports = {ports.Count(p => p.IsOn)}");
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
                foreach (var p in ports) await p.Write(false, force: true);
            }
        }
    }

}
