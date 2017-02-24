# Copyright 2016 The XMPMeta Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################
#
# By default, there is no easy way in CMake to set the value of a cache
# variable without reinitialising it, which involves resetting its
# associated help string.  This is particularly annoying for CMake options
# where they need to programmatically updated.
#
# This function automates this process by getting the current help string
# for the cache variable to update, then reinitialising it with the new
# value, but with the original help string.
function(UPDATE_CACHE_VARIABLE VAR_NAME VALUE)
  get_property(HELP_STRING CACHE ${VAR_NAME} PROPERTY HELPSTRING)
  get_property(VAR_TYPE CACHE ${VAR_NAME} PROPERTY TYPE)
  set(${VAR_NAME} ${VALUE} CACHE ${VAR_TYPE} "${HELP_STRING}" FORCE)
endfunction()
