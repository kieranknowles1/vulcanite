import vulcanite from './vulcanite.js'
import test_vncore from './test-vncore.js'

function createModule(logElement) {
  return {
    print: function(text) {
      logElement.textContent += text + '\n'
    },
    printErr: function(text) {
      logElement.textContent += text + '\n'
    },
  }
}

test_vncore(createModule(document.getElementById('testlog')))
vulcanite(createModule(document.getElementById('log')))
