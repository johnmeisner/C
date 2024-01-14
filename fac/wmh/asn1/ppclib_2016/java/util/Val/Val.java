

import java.io.File;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Source;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

public class Val {

   static boolean hadErrors = false;
   
   /**
    * @param args
    */
   public static void main(String[] args) throws Exception {

      SchemaFactory schemaFactory = SchemaFactory.newInstance(
            javax.xml.XMLConstants.W3C_XML_SCHEMA_NS_URI);
      
      DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
      
      if ( args.length < 2 ) {
         System.out.println("XML Validator");
         System.out.println("Syntax is:");
         System.out.println("\tVal instance schema [schema...]");
         System.out.println("\tVal - schema [schema...]");
         System.out.println();
         System.out.println("The second form validates schemas only.");
         System.out.println("If providing the location of imported schemas,");
         System.out.println(
               "list the imported schemas before the importing schemas.");
         System.out.println();
         System.out.println("You are using:\n\t" + 
               schemaFactory.getClass().getName()+
               "\n\t" + dbFactory.getClass().getName());
         return;
      }
            
      //Create array of Sources
      Source[] schemaSources = new Source[args.length - 1];
      
      for( int i = 1; i < args.length; i++) {
         File file = new File(args[i]);
         if ( file.exists() ) {
            schemaSources[i-1] = new StreamSource(file);
         }
         else {
            //not a file, we'll try it as a URI
            schemaSources[i-1] = new StreamSource(args[i]);
         }
      }
      
      try { 
         //if this suceeds, the schema is valid
         Schema schema = schemaFactory.newSchema(schemaSources);
   
         
         dbFactory.setSchema(schema);
         dbFactory.setNamespaceAware(true);
         
         DocumentBuilder builder = dbFactory.newDocumentBuilder();
         
         builder.setErrorHandler(
              new ErrorHandler() {

               public void error(SAXParseException exception)
                     throws SAXException {
                  report(exception, "error");                  
               }

               public void fatalError(SAXParseException exception)
                     throws SAXException {
                  report(exception, "fatal error");
                  
               }

               public void warning(SAXParseException exception)
                     throws SAXException {
                  report(exception, "warning");
               }
              }
              );
      
         String instance = args[0];
         if (instance.equals("-")) {
            System.out.println("Schema(s) valid.");
         }
         else {
            File instanceAsFile = new File(instance);
   
            if ( instanceAsFile.exists() )
               //instance identifies a file
               builder.parse(instanceAsFile);
            else
               //try instance as a URI
               builder.parse(instance);
            
            if ( !hadErrors) System.out.println("Instance is valid.");
         }
      }
      catch ( SAXParseException e ) {
         report(e, "fatal error");
      }
      
      System.exit( hadErrors ? 1 : 0 );
   }
   
   private static void report(SAXParseException exception, String type) {
     
      System.out.print(type);
      System.out.print(": ");
      System.out.print( exception.getSystemId() );
      System.out.print( " [" );
      System.out.print( exception.getLineNumber() );
      System.out.print( "," ); 
      System.out.print( exception.getColumnNumber() );
      System.out.print( "]: " );
      System.out.println ( exception.getMessage()); 
      
      hadErrors = true;
   }   
   
}
