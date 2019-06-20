using System;

namespace ParameterClass
{
    class Parameters
    {
	// required TestRig parameters	
	// parameters used to gather data
	public int testTimeout = 90000;
	public string useLogic = "none";
	public double sampleTimeMs = 15000;
	public double sampleFrequency = 4000000;
	public bool useCOMPort = true;
	public string forceCOM = "";
	public string COMParameters = "115200,N,8,1";
	public bool useTestScript = false;
	public string testScriptName = "none.tst";
	public int testScriptTimeoutMs = 90000;
	public string useAnalysis = "none";
	public string analysisScriptName = "none.exe";
	public bool useResultsFile = false;
	public string resultsFileName = "results.txt";
	public int testDelay = 0;
	// Do not change text format above this point
    }
}
