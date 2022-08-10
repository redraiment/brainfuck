(defproject me.zzp/brainfuck "0.0.1"
  :description "Brainfuck translator in Clojure macro, as embedded DSL."
  :url "https://github.com/redraiment/brainfuck"
  :dependencies [[org.clojure/clojure "1.11.1"]]
  :main ^:skip-aot me.zzp.brainfuck
  :test-paths ["src"]
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all
                       :jvm-opts ["-Dclojure.compiler.direct-linking=true"]}})
