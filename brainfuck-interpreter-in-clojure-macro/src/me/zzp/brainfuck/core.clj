(ns me.zzp.brainfuck.core
  "Embeded Brainfuck into Clojure"
  (:require [me.zzp.brainfuck.parser :refer [brainfuck]])
  (:gen-class))

(defn -main
  "Hello World! simple"
  [& args]
  ;; This is brainfuck code, and also is clojure code.
  (brainfuck
   ++++++++++
   [
    >+++++++
    >++++++++++
    >+++
    >+
    <<<<-
    ]
   >++. >+. +++++++.. +++. "Hello"
   >++. " "
   <<+++++++++++++++. >. +++. ------. --------. "World"
   >+. >. "!\n"))
