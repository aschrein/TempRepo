import sys , os , commands
src_filename = str( sys.argv[ 1 ] )
try:
	os.mkdir( "modules" )
except Exception:
	pass
defines = ""
function_texts = []
src_file = open( src_filename , "r" )
#assume source file is valid .c file
dmod = 0 #defines mode
fmod = 1 #function mode
skip_p = False #in case \ used at the end of line
cur_mode = -1
cur_function_text = ""
brace_counter = 0
while True:
	c = src_file.read( 1 )
	if not c:
		break
	if c == '#':
		cur_mode = dmod
		
	if cur_mode == dmod:
		defines += c
	else:
		cur_function_text += c
	
	if c == '\n':
		if skip_p:
			skip_p = False
		else:
			cur_mode = fmod
			
	if c == '\\':
		skip_p = True
	
	if c == '}':
		brace_counter -= 1
		if brace_counter == 0:
			function_texts.append( cur_function_text )
			cur_function_text = ""
			continue
	if c == '{':
		brace_counter += 1
src_file.close()
def getDependencies( src_filename ):
	command = """gcc -Iinclude """ + src_filename + """ 2>&1 | grep undefined | sed -r \"s|.*[\\'\`]([^\`\\']+)[\\'\`].*|\\1|\" | grep -v main"""
	output = commands.getoutput( command )
	#print command
	#print output
	if not output or output == "":
		return None
	return output.split( "\n" )
for function_text in function_texts:
	name = function_text.split( "(" )[ 0 ].split( " " )[ 1 ].replace( " " , "" )
	print name
	src_file = "modules/" + name + ".c"
	f = open( src_file , "wb" )
	f.write( defines + function_text )
	f.close()
	dep = getDependencies( src_file )
	if not dep == None:
		f = open( src_file.replace( ".c" , ".dep" ) , "wb" )
		for line in dep:
			f.write( line + "\n" )
		f.close()
	#print 
	

