message("Running revision.cmake")

execute_process(COMMAND hg summary
	OUTPUT_VARIABLE HYDRA_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)

# Necessary to replace line breaks for preprocessor
# Very hard to read, but replaces every line break by escaped line break
# for printing and backslash for preprocessor line break and one regular line
# break for the editor.
string(REPLACE "\n" "\\n\\\n" _output ${HYDRA_REVISION})
file(WRITE revision_defines.hpp "#define HYDRA_REVISION \"${_output}\"")

