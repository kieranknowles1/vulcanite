#!/usr/bin/env nu
def runtest [
  file: string
] {
  (./build/vulcanite
    debug.validation_layers 1
    log.level warn
    debug.fixed_input $"($env.FILE_PWD)/integrationtests/($file)")
}

# Hot loading a scene should not cause any errors
runtest reloadscene.json
