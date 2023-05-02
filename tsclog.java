public class tsclog
{
    public static native int availcpus();
    public static native int cpu();
    public static native int tid();
    public static native void pin(int cpu);
    public static native long now();
    public static native long stdout_now();
    public static native long stdout_label_now(String label);
    public static native long stderr_now();
    public static native long stderr_label_now(String label);
    

    static {
	System.loadLibrary("tsclog");
    }

    public static void main(String[] args)
    {
	int avail = tsclog.availcpus();
	int cpu = tsclog.cpu();
	int tid = tsclog.tid();
	long tsc = tsclog.now();
	
	System.out.println("available cpus: " + avail);
	System.out.println("cpu: " + cpu + " tid: " + tid +
			   " now: " + Long.toUnsignedString(tsc));
	
	tsclog.pin(avail-1);
	cpu = tsclog.cpu();
	tid = tsclog.tid();
	tsc = tsclog.now();
	System.out.println("cpu: " + cpu + " tid: " + tid +
			   " now: " + Long.toUnsignedString(tsc));
    
	tsclog.stdout_now();
	tsclog.stderr_now();	
	tsclog.stdout_label_now("mapper1");
	tsclog.stderr_label_now("mapper2");	
    }
}
