(ns me.zzp.brainfuck
  "Brainfuck translator"
  (:gen-class))

(defn to-tokens
  "Convert codes to tokens"
  [codes]
  (->> codes
    (mapcat str)
    (filter #{\> \< \+ \- \. \, \[ \]})))

(defn to-ast
  "Convert tokens to AST"
  [tokens]
  (-> tokens
    ((fn build [stream]
       (loop [[head & tail] stream
              ast []]
         (case head
           (nil \]) [tail ast]
           \[ (let [[tokens sub-instruction] (build tail)]
                (recur tokens (conj ast sub-instruction)))
           (recur tail (conj ast head))))))
    second))

(defmacro brainfuck
  "Brainfuck translator: translates brainfuck to Clojure."
  [& codes]
  (to-ast (to-tokens codes)))

(defn -main
  "I don't do a whole lot ... yet."
  [& args]
  (macroexpand-1 '(brainfuck ++++++++++
              [
               >+++++++
               >++++++++++
               >+++
               >+
               <<<<-
               ]
              >++. >+. +++++++.. +++.
              >++.
              <<+++++++++++++++. >. +++. ------. --------. >+.
              >.))
)
