using System;
using System.Globalization;
using System.IO;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Threading;
using System.Threading.Tasks;
using static System.FormattableString;

namespace cathouse_analysis
{
    public class Engine
    {
        HttpClient client = new HttpClient();

        public Engine()
        {
            /* client.BaseAddress = new Uri("http://cathouse.searchathing.com");
            client.DefaultRequestHeaders.Accept.Clear();
            client.DefaultRequestHeaders.Accept.Add(
                new MediaTypeWithQualityHeaderValue("application/text"));*/

        }

        async Task<double> GetTBottom()
        {
            var s = await client.GetStringAsync($"http://cathouse.searchathing.com/temp/28b03724070000c8");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }

        async Task<double> GetTAmbient()
        {
            var s = await client.GetStringAsync($"http://cathouse.searchathing.com/temp/28f00a3b05000038)");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }

        async Task<double> GetTWood()
        {
            var s = await client.GetStringAsync($"http://cathouse.searchathing.com/temp/28e2cc23070000d8)");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }

        async Task<double> GetTExtern()
        {
            var s = await client.GetStringAsync($"http://temp-sensors.searchathing.com/temp/28b5742407000084)");
            return double.Parse(s, CultureInfo.InvariantCulture);
        }

        async Task HeatCathouse(bool on)
        {
            await client.GetStringAsync($"http://cathouse.searchathing.com/relay/0/{(on ? "on" : "off")}");
        }

        public async Task Run()
        {
            try
            {
                var power = 120d;
                var wH = 0d; // watt x Hr
                var sw = new StreamWriter("/home/devel0/devel-tmp/cathouse-lab/data.csv", true);
                var dtBegin = DateTime.Now;
                var dt = DateTime.Now;
                DateTime? dton = null;
                var cooldownMin = TimeSpan.FromMinutes(10);
                var dtcooldown = DateTime.Now - cooldownMin;
                var dtLastCycle = DateTime.Now;
                var totOnHr = 0d;
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

                        if (dton.HasValue) // if currently on account power
                        {
                            var hrOn = (DateTime.Now - dtLastCycle).TotalHours;
                            totOnHr += hrOn;
                            wH += power * hrOn;
                        }
                        dtLastCycle = DateTime.Now;
                        var runtime = (DateTime.Now - dtBegin).TotalHours;

                        var str = Invariant($"{(catinthere ? "1" : "0")} {dtstr} {tbottom} {tambient} {twood} {textern}");
                        System.Console.WriteLine(string.Format("{0} bottom:{1,-8} ambient:{2,-8} wood:{3,-8} extern:{4,-8} power(wH):{5,-8:0.00} pmean(w):{6,-8:0.00} runtime(hr):{7,-8:0.000} toton(hr):{8,-8:0.000} on(%):{9,-8:0.000}",
                        dtstr, tbottom, tambient, twood, textern,
                        wH,
                        runtime >= 1.0 ? wH / runtime : wH,
                        runtime,
                        totOnHr,
                        totOnHr / runtime));

                        if ((DateTime.Now - dt).TotalSeconds >= 60) // flush data recording
                        {
                            sw.WriteLine(str);
                            sw.Flush();
                            dt = DateTime.Now;
                        }

                        var needOn = false;

                        if (DateTime.Now - dtcooldown > cooldownMin) // don't enable in cooldown time
                        {
                            // eval if need to enable
                            if (textern < 10)
                            {
                                if (tbottom <= 30 && twood <= 40)
                                    needOn = true;
                            }
                        }

                        if (needOn && !dton.HasValue) // turn on
                        {
                            dton = DateTime.Now;
                            System.Console.WriteLine($"---> ON");
                            await HeatCathouse(true);
                        }
                        else if (!needOn && dton.HasValue) // turn off
                        {
                            dtcooldown = DateTime.Now;

                            System.Console.WriteLine(Invariant($"---> OFF"));
                            dton = null;
                            await HeatCathouse(false);                            
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
                await HeatCathouse(false);
            }
        }
    }

}
