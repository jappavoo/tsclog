import edu.bu.cs.sesa.tsclog.*;

public class testtsclog {
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
