using System;

namespace cathouse_analysis
{

    public class ExcursionSample
    {

        public TimeSpan MaxAge { get; private set; }

        public ExcursionSample(TimeSpan maxAge)
        {
            MaxAge = maxAge;
        }

        /// <summary>
        /// nr. of samples analyzed
        /// </summary>
        /// <value></value>
        public int Count { get; private set; }

        double max;

        /// <summary>
        /// generate exception if Count==0
        /// </summary>
        public double Max
        {
            get
            {
                if (Count == 0) throw new Exception($"can't state Max because sample count = 0");
                return max;
            }
        }

        double min;

        /// <summary>
        /// generate exception if Count==0
        /// </summary>        
        public double Min
        {
            get
            {
                if (Count == 0) throw new Exception($"can't state Max because sample count = 0");
                return min;
            }
        }

        public DateTime OldestSampleTimestamp { get; private set; }

        /// <summary>
        /// time to oldest sample from now
        /// </summary>
        public TimeSpan Age { get { return DateTime.Now - OldestSampleTimestamp; } }

        public bool Expired
        {
            get { return Age > MaxAge; }
        }

        /// <summary>
        /// record given value and set OlderSampleTimestamp if this is the first of the serie
        ///         
        /// returns false is this MeanSample expired
        /// </summary>
        public bool Add(double val)
        {
            if (Count == 0)
            {
                min = max = val;
                OldestSampleTimestamp = DateTime.Now;
            }

            if (Expired) return false;

            min = Math.Min(min, val);
            max = Math.Max(max, val);

            ++Count;

            return true;
        }

    }

}