import java.io.FileOutputStream;
import java.io.ObjectOutputStream;
import java.io.IOException;




public class Sample implements java.io.Serializable {


private Class c;


Sample(String nm)
{
   try {
      c = Class.forName(nm);
    }
   catch (Exception e) { }
}



public static void main(String args[])
{
   Sample s = new Sample("java.util.Enumeration");

   try {
      ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream("t.javaout"));
      oos.writeObject(s);
      oos.close();
      System.out.println("Worked");
    }
   catch (IOException e) {
      System.out.println("Failed with " + e.getMessage());
    }
}


}
