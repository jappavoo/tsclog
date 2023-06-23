package tsc;

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
    public static native long mklog(String name, long n,
				    int valsperentry,
				    int logonexit, int binary,
				    String valhdrs);
    public static native void log(long lptr);
    public static native void log1(long lptr, long v1);
    
    private long logptr = 0;
    
    static {
	System.loadLibrary("tsclog");
    }

    public tsclog(String name, long n, int valsperentry, String valhdrs) {
	logptr = mklog(name,n,valsperentry,
		       1,  // logonexit
		       0,  // binary
		       valhdrs
		       );
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

	tsclog.stdout_label_now("pre pin");
	tsclog.pin(avail-1);
	tsclog.stdout_label_now("post pin");
	cpu = tsclog.cpu();
	tid = tsclog.tid();
	tsc = tsclog.now();
	System.out.println("cpu: " + cpu + " tid: " + tid +
			   " now: " + Long.toUnsignedString(tsc));
    
	tsclog.stdout_now();
	tsclog.stderr_now();	
	tsclog.stdout_label_now("mapper1");
	tsclog.stderr_label_now("mapper2");
	
	tsclog log = new tsclog("",10,0,"");
	for (int i=0; i<10; i++) {
	    log.log();
	}

	tsclog log1 = new tsclog("JAVALOG", 10,1,"i");
	for (int i=0; i<10; i++) {
	    log1.log1(i);
	}

    }
}
