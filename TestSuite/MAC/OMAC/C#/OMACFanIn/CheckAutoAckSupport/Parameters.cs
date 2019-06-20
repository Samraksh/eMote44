using System;

namespace ParameterClass
{
    class Parameters
    {
		// required TestRig parameters	
		// parameters used to gather data
		// 3 mins is 0.05; 30 mins is 0.5;
		public int testTimeout = 1800000;    //0.5*60*60*1000 (30 mins in ms)
		public string useLogic = "none";
		public double sampleTimeMs = 15000;
		public double sampleFrequency = 4000000;
		public bool useCOMPort = false;
		public string forceCOM = "";
		public string COMParameters = "115200,N,8,1";
		public bool useTestScript = false;
		public string testScriptName = "BasicPing.tst";
		public int testScriptTimeoutMs = 10000;
		public string useAnalysis = "none";
		public string analysisScriptName = "none.exe";
		public bool useResultsFile = false;
		public string resultsFileName = "results.txt";
		// Do not change text format above this point
    }
}
