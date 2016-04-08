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
            
            
            if command=="bookmark" || command=="book mark" {
                task.launchPath = "/usr/bin/osascript"
                task.arguments = ["./bookmark.scpt"]
                task.launch()

            }
            
            if command=="enlarge" {
                task.launchPath = "/usr/bin/osascript"
                task.arguments = ["./enlarge.scpt"]
                task.launch()
                
            }
            
            if command=="shrink" {
                task.launchPath = "/usr/bin/osascript"
                task.arguments = ["./shrink.scpt"]
                task.launch()
                
            }
            
            if command=="next page" {
                task.launchPath = "/usr/bin/osascript"
                task.arguments = ["./next.scpt"]
                task.launch()
                
            }
            
            if command=="last page" {
                task.launchPath = "/usr/bin/osascript"
                task.arguments = ["./back.scpt"]
                task.launch()
                
            }
            
            if command=="open the book" {
                task.launchPath = "/usr/bin/osascript"
                task.arguments = ["./open.scpt"]
                task.launch()
                
            }
            
            if command=="close the book" {
                task.launchPath = "/usr/bin/osascript"
                task.arguments = ["./close.scpt"]
                task.launch()
                
            }
            
            
    }
}

let dispatcher = Dispatcher()

let recognizer = NSSpeechRecognizer()!
recognizer.delegate = dispatcher
recognizer.commands = ["bookmark","book mark", "enlarge", "shrink", "next page", "last page", "open the book", "close the book"]
recognizer.startListening()

let loop = NSRunLoop.currentRunLoop()
let mode = loop.currentMode ?? NSDefaultRunLoopMode
while loop.runMode(mode, beforeDate: NSDate(timeIntervalSinceNow: 0.1))
    && !dispatcher.stop {}