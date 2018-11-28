using System;
using System.Net.Http;
using System.Threading.Tasks;
using System.Net.Http.Headers;
using System.Globalization;

namespace cathouse_analysis
{
    class Program
    {

        static void Main(string[] args)
        {
            var engine = new Engine();

            engine.Run().Wait();
        }
    }
}
