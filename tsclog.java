public class tsclog
{
    public static native int init();
    public static native int log();
    public static native int done();

    static {
	System.loadLibrary("tsclog");
    }

    public static void main(String[] args)
    {
	int a=tsclog.init();
	int b=tsclog.log();
	int c=tsclog.done();
	System.out.println("init: " + a);
	System.out.println("log: " + b);
	System.out.println("done: " + c);
    }
}
