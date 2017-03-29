#!/usr/bin/env bash

set -e


MY_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo $MY_DIR

cd "${MY_DIR}"
cd ..

GENIE=include/genie/bin/release/genie
PREMAKE=include/premake/bin/release/premake5



echo dofile \"build/genie/genie.lua\" > genie.lua

${GENIE} --target="windesktop81" vs2015 || goto err
${GENIE} --target="winphone8"    vs2012 || goto err
${GENIE} --target="winphone81"   vs2013 || goto err
${GENIE} --target="winstore81"   vs2013 || goto err
${GENIE} --target="winstore82"   vs2015 || goto err

rm genie.lua



echo dofile \"build/premake/premake.lua\" > premake5.lua

${PREMAKE} --group=libopenmpt_test vs2010
${PREMAKE} --group=foo_openmpt vs2010
${PREMAKE} --group=in_openmpt vs2010
${PREMAKE} --group=xmp-openmpt vs2010
${PREMAKE} --group=libopenmpt-small vs2010
${PREMAKE} --group=libopenmpt vs2010
${PREMAKE} --group=openmpt123 vs2010
#${PREMAKE} --group=PluginBridge vs2010
#${PREMAKE} --group=OpenMPT-VSTi vs2010
#${PREMAKE} --group=OpenMPT vs2010
${PREMAKE} --group=all-externals vs2010

${PREMAKE} --group=libopenmpt_test vs2012
${PREMAKE} --group=foo_openmpt vs2012
${PREMAKE} --group=in_openmpt vs2012
${PREMAKE} --group=xmp-openmpt vs2012
${PREMAKE} --group=libopenmpt-small vs2012
${PREMAKE} --group=libopenmpt vs2012
${PREMAKE} --group=openmpt123 vs2012
#${PREMAKE} --group=PluginBridge vs2012
#${PREMAKE} --group=OpenMPT-VSTi vs2012
#${PREMAKE} --group=OpenMPT vs2012
${PREMAKE} --group=all-externals vs2012

${PREMAKE} --group=libopenmpt_test vs2013
${PREMAKE} --group=foo_openmpt vs2013
${PREMAKE} --group=in_openmpt vs2013
${PREMAKE} --group=xmp-openmpt vs2013
${PREMAKE} --group=libopenmpt-small vs2013
${PREMAKE} --group=libopenmpt vs2013
${PREMAKE} --group=openmpt123 vs2013
#${PREMAKE} --group=PluginBridge vs2013
#${PREMAKE} --group=OpenMPT-VSTi vs2013
#${PREMAKE} --group=OpenMPT vs2013
${PREMAKE} --group=all-externals vs2013

${PREMAKE} --group=libopenmpt_test vs2015
${PREMAKE} --group=foo_openmpt vs2015
${PREMAKE} --group=in_openmpt vs2015
${PREMAKE} --group=xmp-openmpt vs2015
${PREMAKE} --group=libopenmpt-small vs2015
${PREMAKE} --group=libopenmpt vs2015
${PREMAKE} --group=openmpt123 vs2015
${PREMAKE} --group=PluginBridge vs2015
${PREMAKE} --group=OpenMPT-VSTi vs2015
${PREMAKE} --group=OpenMPT vs2015
${PREMAKE} --group=all-externals vs2015

${PREMAKE} --group=libopenmpt_test vs2017
${PREMAKE} --group=foo_openmpt vs2017
${PREMAKE} --group=in_openmpt vs2017
${PREMAKE} --group=xmp-openmpt vs2017
${PREMAKE} --group=libopenmpt-small vs2017
${PREMAKE} --group=libopenmpt vs2017
${PREMAKE} --group=openmpt123 vs2017
${PREMAKE} --group=PluginBridge vs2017
${PREMAKE} --group=OpenMPT-VSTi vs2017
${PREMAKE} --group=OpenMPT vs2017
${PREMAKE} --group=all-externals vs2017

${PREMAKE} --group=libopenmpt_test vs2010 --xp
${PREMAKE} --group=foo_openmpt vs2010 --xp
${PREMAKE} --group=in_openmpt vs2010 --xp
${PREMAKE} --group=xmp-openmpt vs2010 --xp
${PREMAKE} --group=libopenmpt-small vs2010 --xp
${PREMAKE} --group=libopenmpt vs2010 --xp
${PREMAKE} --group=openmpt123 vs2010 --xp
#${PREMAKE} --group=PluginBridge vs2010 --xp
#${PREMAKE} --group=OpenMPT-VSTi vs2010 --xp
#${PREMAKE} --group=OpenMPT vs2010 --xp
${PREMAKE} --group=all-externals vs2010 --xp

${PREMAKE} --group=libopenmpt_test vs2012 --xp
${PREMAKE} --group=foo_openmpt vs2012 --xp
${PREMAKE} --group=in_openmpt vs2012 --xp
${PREMAKE} --group=xmp-openmpt vs2012 --xp
${PREMAKE} --group=libopenmpt-small vs2012 --xp
${PREMAKE} --group=libopenmpt vs2012 --xp
${PREMAKE} --group=openmpt123 vs2012 --xp
#${PREMAKE} --group=PluginBridge vs2012 --xp
#${PREMAKE} --group=OpenMPT-VSTi vs2012 --xp
#${PREMAKE} --group=OpenMPT vs2012 --xp
${PREMAKE} --group=all-externals vs2012 --xp

${PREMAKE} --group=libopenmpt_test vs2013 --xp
${PREMAKE} --group=foo_openmpt vs2013 --xp
${PREMAKE} --group=in_openmpt vs2013 --xp
${PREMAKE} --group=xmp-openmpt vs2013 --xp
${PREMAKE} --group=libopenmpt-small vs2013 --xp
${PREMAKE} --group=libopenmpt vs2013 --xp
${PREMAKE} --group=openmpt123 vs2013 --xp
#${PREMAKE} --group=PluginBridge vs2013 --xp
#${PREMAKE} --group=OpenMPT-VSTi vs2013 --xp
#${PREMAKE} --group=OpenMPT vs2013 --xp
${PREMAKE} --group=all-externals vs2013 --xp

${PREMAKE} --group=libopenmpt_test vs2015 --xp
${PREMAKE} --group=foo_openmpt vs2015 --xp
${PREMAKE} --group=in_openmpt vs2015 --xp
${PREMAKE} --group=xmp-openmpt vs2015 --xp
${PREMAKE} --group=libopenmpt-small vs2015 --xp
${PREMAKE} --group=libopenmpt vs2015 --xp
${PREMAKE} --group=openmpt123 vs2015 --xp
${PREMAKE} --group=PluginBridge vs2015 --xp
${PREMAKE} --group=OpenMPT-VSTi vs2015 --xp
${PREMAKE} --group=OpenMPT vs2015 --xp
${PREMAKE} --group=all-externals vs2015 --xp

${PREMAKE} --group=libopenmpt_test vs2017 --xp
${PREMAKE} --group=foo_openmpt vs2017 --xp
${PREMAKE} --group=in_openmpt vs2017 --xp
${PREMAKE} --group=xmp-openmpt vs2017 --xp
${PREMAKE} --group=libopenmpt-small vs2017 --xp
${PREMAKE} --group=libopenmpt vs2017 --xp
${PREMAKE} --group=openmpt123 vs2017 --xp
${PREMAKE} --group=PluginBridge vs2017 --xp
${PREMAKE} --group=OpenMPT-VSTi vs2017 --xp
${PREMAKE} --group=OpenMPT vs2017 --xp
${PREMAKE} --group=all-externals vs2017 --xp

${PREMAKE} postprocess

rm premake5.lua

