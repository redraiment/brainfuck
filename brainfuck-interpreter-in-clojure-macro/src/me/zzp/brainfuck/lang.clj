(ns me.zzp.brainfuck.lang
  "Language package: implement brainfuck SIX commands in Clojure.
  See also https://en.wikipedia.org/wiki/Brainfuck#Commands"
  (:refer-clojure :exclude [get set update zero?]))

(def ^:dynamic *ds*
  "Data Segment"
  nil)

(def ^:dynamic *dp*
  "Data Pointer"
  nil)

(defn- index
  "Obtain value of data pointer"
  []
  @*dp*)

(defn- get
  "Read the value at data pointer"
  []
  (aget *ds* (index)))

(defn- set
  "Save the value to data pointer"
  [value]
  (aset-byte *ds* (index) value))

(defn- update
  "Update the value at data pointer by specified function"
  [f & args]
  (set (apply f (get) args)))

(defn forward
  "Command `>`: increment the data pointer"
  []
  (swap! *dp* inc))

(defn backword
  "Command `<`: decrement the data pointer"
  []
  (swap! *dp* dec))

(defn increment
  "Command `+`: increment the value at the data pointer"
  []
  (update inc))

(defn decrement
  "Command `-`: decrement the value at the data pointer"
  []
  (update dec))

(defn input
  "Command `,`: accept one byte of input, storing its value at the data pointer"
  []
  (set (.read System/in)))

(defn output
  "Command `.`: output the byte at the data pointer"
  []
  (.write System/out (byte (get))))

(defn zero?
  "Condition of command `[`: check the byte at the data pointer is zero"
  []
  (clojure.core/zero? (get)))
