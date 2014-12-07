
BEGIN {
  print "#ifndef SUPPORT_HXX"
  print "#define SUPPORT_HXX"

  print "\nconst char* basic_ir_support_library {"
}

/^;/ { next }
/^\s*$/ { next }
{ 
    gsub(/\\/, "\\\\", $0)
    gsub(/\"/, "\\\"", $0)
    print "  \"" $0 "\\n\"" 
}

END {
  print "};\n"
  print "#endif\n"
}

