// OSX NSSpeechRecognizer example
import Foundation
import AppKit


class Dispatcher:  NSObject, NSSpeechRecognizerDelegate {
    var stop: Bool
    override init () {stop = false}
    func speechRecognizer(sender: NSSpeechRecognizer,
        didRecognizeCommand command: String) {
            print("command: \(command)")
            //stop = true
            let task = NSTask()
            task.launchPath = "/usr/bin/osascript"
            task.arguments = ["./bookmark.scpt"]
            
            task.launch()
    }
}

let dispatcher = Dispatcher()

let recognizer = NSSpeechRecognizer()!
recognizer.delegate = dispatcher
recognizer.commands = ["bookmark", "book mark"]
recognizer.startListening()

let loop = NSRunLoop.currentRunLoop()
let mode = loop.currentMode ?? NSDefaultRunLoopMode
while loop.runMode(mode, beforeDate: NSDate(timeIntervalSinceNow: 0.1))
    && !dispatcher.stop {}