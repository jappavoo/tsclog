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
    public static native long mklog(long n);
    public static native void log(long lptr);
    public static native void log1(long lptr, long v1);
    
    private long logptr = 0;
    
    static {
	System.loadLibrary("tsclog");
    }

    public tsclog(long n) {
	logptr = mklog(n);
	System.out.println("logptr: 0x" + Long.toUnsignedString(logptr,16));
    }

    public void log() {
	tsclog.log(logptr);

    }

    public void log1(long v1) {
	tsclog.log1(logptr,v1);
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
	
	tsclog log = new tsclog(1000);
	
    }
}
