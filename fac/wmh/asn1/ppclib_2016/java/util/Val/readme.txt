Val validates an XML instance against a schema.
The command to use is:
java [options] Val instance schema [schema...]

options: Java options, such as -cp to set the classpath.

instance: The instance to be validated.  Use a single dash ("-") to only
validate schemas.

schema: One or more schema specifications.

Val will first try to resolve its arguments as file names.  If an argument
is not valid as a file name, Val will try using it as a URI.

If you invoke Val without arguments, it will report the class names
of the SchemaFactory and the DocumentBuilderFactory, indicating what
JAXP implementation is in use.

Imported Schemas:
JAXP will attempt to use the schemaLocation given within an <import> to locate
the imported schema.  However, if the schemaLocation is not specified, or is
not resolvable, you can also include imported schemas on the command line.
In this case, just be sure to list the imported schemas prior to the importing
schema, or JAXP will not be able to parse the importing schema.
