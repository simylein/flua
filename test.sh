#!/bin/sh

blue="\033[34m"
cyan="\033[36m"
red="\033[31m"
green="\033[32m"
bold="\033[1m"
normal="\033[22m"
reset="\033[39m"

passed="0"
failed="0"

info() {
  echo "${bold}${blue}test${cyan} info ${reset}${normal}${1}"
}

pass() {
  echo "${bold}${blue}test${green} pass ${reset}${1}${normal}"
}

fail() {
  echo "${bold}${blue}test${red} fail ${reset}${1}${normal}"
}

test() {
  local success="true"

  if [ -z "${body}" ]; then
    local headers="$(curl -s -I -X "${1}" -H "${cookie}" "localhost:2254${2}")"
  else
    local headers="$(printf "${body}" | curl -s -i -X "${1}" -H "${cookie}" "localhost:2254${2}" --data-binary @-)"
  fi

  local status_code=$(echo "${headers}" | grep -i 'http/1.1' | awk '{print $2}')
  if [ "${status_code}" != "${3}" ]; then
    success="false"
    info "status-code: expected ${green}${3}${reset} received ${red}${status_code}${reset}"
  fi

  if [ "${4}" != "" ]; then
    local content_type=$(echo "${headers}" | grep -i 'content-type:' | cut -d ':' -f2 | tr -d '[:space:]')
    if [ "${content_type}" != "${4}" ]; then
      success="false"
      info "content-type: expected ${green}${4}${reset} received ${red}${content_type}${reset}"
    fi
  fi

  if [ "${5}" != "" ]; then
    local content_length=$(echo "${headers}" | grep -i 'content-length:' | cut -d ':' -f2 | tr -d '[:space:]')
    if [ "${content_length}" != "${5}" ]; then
      success="false"
      info "content-length: expected ${green}${5}${reset} received ${red}${content_length}${reset}"
    fi
  fi

  if [ "${6}" != "" ]; then
    local location=$(echo "${headers}" | grep -i 'location:' | cut -d ':' -f2 | tr -d '[:space:]')
    if [ "${location}" != "${6}" ]; then
      success="false"
      info "location: expected ${green}${6}${reset} received ${red}${location}${reset}"
    fi
  fi

  local set_cookie=$(echo "${headers}" | grep -i 'set-cookie:' | cut -d ':' -f2 | tr -d '[:space:]' | grep -i 'auth=' | cut -d ';' -f1)
  if [ "${set_cookie}" != "" ]; then
    export cookie="cookie:${set_cookie}"
  fi

  if [ ${success} = "true" ]; then
    pass "${1} ${2}"
    passed=$((${passed} + 1))
  else
    fail "${1} ${2}"
    failed=$((${failed} + 1))
  fi
}

test "get" "/" 200 "text/html"
test "get" "/?hello=world" 400 "text/html"

test "get" "/signup" 200 "text/html"
test "get" "/signup?hello=world" 400 "text/html"

test "get" "/signin" 200 "text/html"
test "get" "/signin?hello=world" 400 "text/html"

test "get" "/testing" 404

test "get" "/api/me" 401

test "delete" "/api/me" 401

test "post" "/api/signin" 400
export body="testing\0.go4Testing\0"
test "post" "/api/signin" 401
unset body

test "post" "/api/signup" 400
export body="testing\0.go4Testing\0"
test "post" "/api/signup" 201
unset body

test "get" "/api/me" 200 "application/octet-stream" "25"

test "get" "/api/year" 400
test "get" "/api/year?user=testing" 200 "application/octet-stream" "0"

test "get" "/api/flight" 400
test "get" "/api/flight?user=testing&year=1970" 200 "application/octet-stream" "0"

test "post" "/api/flight" 400
export body="EzoXuI6E3Fkg86HQXu6KZNnNaS1xRNiV\0\0\0\0\0\0\1\2\0\0\0\0\0\0\1\6\0\1\2\3\4\5\6\7\8\0\1\2\3\4\5\6\7\8"
test "post" "/api/flight" 201
unset body

test "post" "/api/flight" 400
export body="EzoXuI6E3Fkg86HQXu6KZNnNaS1xRNiV\0\0\0\0\0\0\1\2\0\0\0\0\0\0\1\6\0\1\2\3\4\5\6\7\8\0\1\2\3\4\5\6\7\8"
test "post" "/api/flight" 409
unset body

test "get" "/api/year?user=testing" 200 "application/octet-stream" "2"

test "get" "/api/flight?user=testing&year=1970" 200 "application/octet-stream" "36"

test "get" "/testing" 200 "text/html"
test "get" "/testing?year=1970" 200 "text/html"

test "delete" "/api/me" 200
test "delete" "/api/me" 500

unset cookie

test "get" "/api/me" 401

test "delete" "/api/me" 401

test "get" "/api/year?user=testing" 404

test "get" "/api/flight?user=testing&year=1970" 404

if [ ${passed} -gt 0 ]; then
  info "${bold}${green}${passed} passed${reset}${normal}"
fi
if [ ${failed} -gt 0 ]; then
  info "${bold}${red}${failed} failed${reset}${normal}"
fi

info "${bold}ran $((${passed} + ${failed})) tests${normal}"

if [ ${failed} -gt 0 ]; then
  exit 1
fi

exit 0
