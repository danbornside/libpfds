# This file is part of libpfds, Persistent Functional Data Structures in C.
#
# libpfds is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# libpfds is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# libpfds. If not, see <https://www.gnu.org/licenses/>.


# a really minimal configure script that does a little bit of what AutoConf
# does but without the autoconf.  really it just parses the options given and
# writes a couple of files with the result of those options.  in particular it
# will make:
#
# config.status a normalized and tidy recreation of the options passed.
# config.inc.sh the relevant options in a form suitable for use in shell programs.
# Makefile.config the relevant options in a makefile, mostly useful for install dirs.

# vim: set syntax=bash sw=4 ts=4 et :

declare -a allPathOptions
declare -A pathOptions
declare -A pathOptionDefaults
declare -A configPathOptions

declare -a allFeatureOptions
declare -A featureOptions
declare -A featureOptionDefaults
declare -A configFeatureOptions

declare -A configEnv

addPathOption () {
    pathVarName="$1"
    pathVarValue="$2"
    pathVarHelp="$3"
    allPathOptions+=( "$pathVarName" )
    pathOptionDefaults["$pathVarName"]="$pathVarValue"
    pathOptions["$pathVarName"]="$pathVarHelp"
}

truthy () {
    case $1 in
        enabled | enable | true | yes | 1 | t | y )
            echo "1"
            ;;
        disabled | disable | false | no | 0 | f | n)
            echo "0"
            ;;
        *)
            echo ""
            ;;
    esac
}


addFeatureOption () {
    featureName="$1"
    featureDefault="$2"
    featureHelp="$3"
    allFeatureOptions+=( "$featureName" )
    featureOptions["$featureName"]="$featureHelp"
    featureOptionDefaults["$featureName"]="$(truthy "$featureDefault")"
}

addPackageOption () {
    echo "featureOption TODO"
    exit 1
}

printVersion () {
    echo "a configure script v0.0.0"
    exit 0
}

printUsage () {
    msg=$1
    if [ "$msg" ] ; then echo "$msg" ; fi
cat << EOF
Usage:
  $0 [OPTION...]

Installation options:
EOF

for v in ${allPathOptions[@]} ; do
    printf '  --%s=PATH   \t%s\n' "$v" "${pathOptions[$v]}"
done

cat << EOF

Features:
  features can be enabled with --enable-x or disabled with --disable-x
EOF

for v in "${allFeatureOptions[@]}" ; do
    printf '  --enable-%s   \t%s\n' "$v" "${featureOptions[$v]}"
done

cat << EOF

Other options:
  -h,--help Show help options
  -v,--version  Show configure version
EOF
}

featureStatus() {
    featureName=$1
    featureDefVal="${featureOptionDefaults[$featureName]}"
    featureVal="${configFeatureOptions[$featureName]:-${featureDefVal}}"
    echo "$featureVal"
}

haveFeature () {
    test "0$(featureStatus "$1")" -eq 1
    return $?
}

handleFeatureOption () {
    featureName="$1"
    if [ "z${featureOptions[$featureName]}" == "z" ] ; then
        printUsage "invalid feature: $featureName"
        exit 1
    fi
    featureValue=$(truthy "$2")
    if [ "z$featureValue" == "z" ] ; then
        printUsage "cant parse truth value of $featureName=$2 (should be enabled|disabled)"
        exit 1
    fi
    configFeatureOptions[$featureName]=$featureValue
}

handlePackageOption () {
    echo "package TODO"
    exit 1
}
handleOtherOption () {
    optName="$1"
    optValue="$2"
    if [ "z${pathOptions["$optName"]}" != "z" ] ; then
        configPathOptions["$optName"]=$optValue
    else
        printUsage "invalid option: $optName"
        exit 1
    fi
}

handleEnvVar () {
    configEnv["$1"]="$2"
}

generateConfigStatus () {
    printf '%s' "$0"
    for v in "${!configEnv[@]}" ; do
        printf ' \\\n\t%s=%s' "$v" "${configEnv[$v]@Q}"
    done
    for v in "${!configPathOptions[@]}" ; do
        printf ' \\\n\t--%s=%s' "$v" "${configPathOptions[$v]@Q}"
    done
    for v in "${!configFeatureOptions[@]}" ; do
        printf ' \\\n\t--%s=%s' "$v" "${configFeatureOptions[$v]@Q}"
    done
    printf '\n'
}

# usage: pathStatus [OPTIONS] varName
#  print configured path with default template handling.
#
# options:
#   -s       adjust template braces for shell (DEFAULT)
#   -m       adjust template braces for makefile
pathStatus () {
    lbrace='${'
    rbrace='}'
    while getopts 'sm' o ; do
        case $o in
            s)
                lbrace='${'
                rbrace='}'
                ;;
            m)
                lbrace='${'
                rbrace='}'
                ;;
            *)
                echo "bad option to pathStatus" >&2
                exit 1
        esac
    done

    pathVarName="$1"
    pathDefVal0="${pathOptionDefaults[$v]}"
    pathDefVal1="${pathDefVal0//'('/${lbrace}}"
    pathDefVal2="${pathDefVal1//')'/${rbrace}}"
    pathVal="${configPathOptions[$v]:-${pathDefVal2}}"
    echo "${pathVal}"
}

envVarStatus () {
    echo "${configEnv[$1]}"
}

# usage:
#   generateConfigInc [OPTIONS] > config.inc
#
# options:
#   -e       prefix var names with 'export'
generateConfigInc () {
    exportPrefix=''
    while getopts 'ex:' o ; do
        case $o in
            e) exportPrefix="export " ;;
            *)
                echo "bad option to generateConfigInc" >&2
                exit 1
        esac
    done

    echo "# generated by $0"
    for v in ${!configEnv[@]} ; do
        printf '%s%s=%s\n' "$exportPrefix" "${v}" "${configEnv[$v]@Q}"
    done
    for v in ${allPathOptions[@]} ; do
        printf '%s%s="%s"\n' "$exportPrefix" "${v}" "$(pathStatus "$v")"
    done
    for v in ${allFeatureOptions[@]} ; do
        printf '%sFEATURE_%s=%d\n' "$exportPrefix" "${v@U}" "$(featureStatus "$v")"
    done
}

generateMakefileInc () {
    echo "# generated by $0"
    for v in ${!configEnv[@]} ; do
        printf '%s = %s\n' "${v}" "${configEnv[$v]@Q}"
    done
    for v in ${allPathOptions[@]} ; do
        printf '%s = "%s"\n' "${v}" "$(pathStatus -m "$v")"
    done
    for v in ${allFeatureOptions[@]} ; do
        if [ "0$(featureStatus "$v")" -eq "1" ] ; then
            printf 'FEATURE_%s = %d\n'  "${v@U}" "$(featureStatus "$v")"
        fi
    done
}

parseConfigureOptions () {
    while [ $# -gt 0 ] ; do
        arg="$1"
        shift
        case "$arg" in
            -v|--version)
                printVersion
                exit 0
                ;;
            --help)
                printUsage
                exit 0
                ;;
            --enable-*=*)
                argNoDashes="${arg#--enable-}"
                argName="${argNoDashes%%=*}"
                argValue="${arg#*=}"
                handleFeatureOption "$argName" "$argValue"
                ;;
            --with-*=*)
                argNoDashes="${arg#--with-}"
                argName="${argNoDashes%%=*}"
                argValue="${arg#*=}"
                handlePackageOption "$argName" enable "$argValue"
                ;;
            --*=*)
                argNoDashes="${arg#--}"
                argName="${argNoDashes%%=*}"
                argValue="${arg#*=}"
                handleOtherOption "$argName" "$argValue"
                ;;
            --disable-*)
                argName="${arg#--disable-}"
                handleFeatureOption "$argName" disable
                ;;
            --enable-*)
                argName="${arg#--enable-}"
                handleFeatureOption "$argName" enable
                ;;
            --without-*)
                argName="${arg#--without-}"
                handlePackageOption "$argName" disable
                ;;
            --with-*)
                argName="${arg#--with-}"
                argValue=$1
                shift
                handlePackageOption "$argName" enable "$argValue"
                ;;

            --*)
                argName="${arg#--}"
                argValue=$1
                shift
                handleOtherOption "$argName" "$argValue"
                ;;
            *=*)
                argName="${arg%%=*}"
                argValue="${arg#*=}"
                handleEnvVar "$argName" "$argValue"
                ;;
            *)
                printUsage "invalid option: $arg" ;;
        esac
    done
}

# vim: set syntax=bash sw=4 ts=4 et :
