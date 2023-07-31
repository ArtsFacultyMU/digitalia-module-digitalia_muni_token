# Digitalia Token

Digitalia token module is created to avoid the classical token calling when 
references are used. In such case the classical token calling cannot iterate 
through multiple values (if there are any) and returs just one value. And if 
we need all nodes/taxonomies that references specific node within this node
we cannot achives this, so Digitalia Token can help in such case. The same 
trouble can cause double fields and triple fields. 

The use cases:

1. Media files

Used to list of specific media type documents that references the processed node.
In the configuration there must be specified media types which should be listed
whenever token [digitalia\_muni:media\_urls] is used. Then all medias (their 
absolute URLs) associated to the processed node are listed. 


2. Reference fields
 
Reference fields might be two types: node and taxonomy. The node type is 
default and has no prefix, the taxonomy type must be prefixed "taxonomy\_\_".

It is generaly configured in format 

ref\_type:ref\_content\_type[:ref\_ref\_content\_type]

in the module configuration (/admin/config/search/digitalia\_muni\_token).
However, the specific configuration will be exaplained for each of the variants 
bellow.


a. Forward references

A forward reference in a node that has a field with name "ref\_type". This 
field is an Entity reference type, pointing to some node(s).

For example a serial might wnat to link to another serial which is its
follower. So there is a "field\_next\_series" which is an Entity reference
to the serial Content type. And there can be multiple referenced serials 
(so the "field\_next\_series" can have multiple values) and all of them will 
be processed by Digitalia Token.

We can extract all necessary (multiple) fields such referenced serial. 
For example we want to know all of the variant titles of such referenced 
serial. The classical token would return just one (first) variant title and 
if we want to get all of them we have to number them specifically which is 
very cumbersome (especially if we don't know how many of them will be there 
in the future).

The token would looks like 

[digitalia\_muni:field\_next\_series:field\_title\_variant]

So what goes after "ref\_type" is a classical token specification, a name of 
the field which stores variant titles in this case.

The token configuration is simple then, just specify 

field\_next\_series:whatever

means that whatever must be there and is tested for being field in the 
referenced entity (see 2nd level bellow), in this case this can be anything,
for example __ or just a name of content type (e.g. serial).


But what if we need to go into 2nd level descendant? For example to get the 
license codes that is valid for the next in series serial, but the license is
an Entity reference to a licence Content type.  We can do so, because
Digitalia Token checks the "ref\_content\_type" (here field\_title\_variant)
for a type, and if the type is Entity reference, than all nodes of this
references is processed. 

[digitalia\_muni:field\_next\_series:field\_next\_series:field\_license:field\_license\_code] 

field\_license is a reference to the license Entity and field\_license\_code is a classical
token (to get the text value of the field).


b. Node referenced by

In case the node is referenced by (an)other node(s), Digitalia Token works 
different way. When defining allowed token with "ref\_type" and this 
"ref\_type" is not a defined field within the processed node, than Digitalia 
Token tries to find all nodes that reference processed node. 

To specify this, the "ref\_type" is a name of the field in (a) node(s) that 
references the processed node and "ref\_content\_type" is a machine name of 
the Content type of (a) node(s) we want to find and which reference(s) the 
processed node.

For example we have a node of Content type "cinema". Each cinema has a 
location, which is represented by another Content type "cinema\_location".
Each cinema\_location has a field "field\_cinema" that references all 
cinemas on that location together with time period (so one cinema can have
more locations depends on time period). The configuration to allow this 
simply looks like:

field\_cinema:cinema\_location

So if we want to know all addresses (stored in the Content type 
"field\_location" in the field "field\_address" which is a plain text)
that the specific cinema has or had in the past, we can write a token this
way:

[digitalia\_muni:digitalia\_muni:field\_cinema:cinema\_location:field\_address]

c. Taxonomy terms

This must be prefixed with taxonomy\_\_ and a field in the processed node
that references a taxonomy. The rest is the same as the a. Forward references.
The example of the configuration 

taxonomy\_\_field\_language:language

and the related token

[digitalia\_muni:taxonomy\_\_field\_language:language:field\_code]


3. Double and triple fields

To access the only specific field of the Content types of Double and Triple 
fields, Digitalia Token can help. At first we need to configure what fields
are allowed together with allowed index (default 1). For example

double\_\_field\_title\_main:1 

Then it might be accessed via 

[digitalia\_muni:double\_\_field\_title\_main:1]

notice that there must be one of the prefixes "double\_\_" or "triple\_\_"
specified when used in token!

## Digitalia Token configuration

/admin/config/search/digitalia\_muni\_token

It is neccessary to define allowed tokens. If any token which is not specified 
is used then warning is logged. 


