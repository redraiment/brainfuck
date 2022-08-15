(ns me.zzp.brainfuck.parser
  "Parser: translate Brainfuck to Clojure."
  (:require [me.zzp.brainfuck.lang :as bf]))

(defn- lexical-analysis
  "Scan code stream and keep EIGHT commands only"
  [stream]
  (->> stream
    (mapcat str)
    (filter #{\> \< \+ \- \. \, \[ \]})))

(defn syntax-analysis
  "Scan command stream and build AST"
  [stream]
  (loop [[head & tail] stream
         ast []]
    (case head
      (nil \]) [ast tail]
      \[ (let [[command tokens] (syntax-analysis tail)]
           (recur tokens (conj ast command)))
      (recur tail (conj ast head)))))

(defn parse
  "Frontend: translate brainfuck to ast (IR)"
  [stream]
  (-> stream
    lexical-analysis
    syntax-analysis
    first))

(defn transform
  "Backend: translate ast to clojure"
  [ast]
  (map #(case %
          \> `(bf/forward)
          \< `(bf/backword)
          \+ `(bf/increment)
          \- `(bf/decrement)
          \. `(bf/output)
          \, `(bf/input)
          `(while (not (bf/zero?))
             ~@(transform %)))
       ast))

(defmacro brainfuck
  "Brainfuck translator: brainfuck to Clojure"
  [& codes]
  `(binding [bf/*ds* (make-array Byte/TYPE 30000)
             bf/*dp* (atom 0)]
     ~@(transform (parse codes))))
