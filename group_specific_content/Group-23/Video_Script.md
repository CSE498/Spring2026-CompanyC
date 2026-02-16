## Overview/Intro:

"Our team (23) is all things Data Analytics. We track what happens as the world runs; agent actions, world changes, key metrics, etc. Then turn we turn that data into post-run insights/ visualizations for debugging, fine tuning, and optimization.  
We support five core classes in our analytics layer: ActionLog, DataLog, Timer, ReplayDriver, and Output Manager."

## Timer (Lauren):

Timer is the part of our analytics layer that measures runtime(stopwatch sytem). It's name based, so we can time specific sections like world updates, agent decisions, or rendering steps without mixing them together. Each time you stop a timer, it records the duration and updates simple summary stats: count, last, min, max, and avg. Then later, those numbers can be reported out for performance issues or debugging. Right now, Timer primarily supports developers through performance profiling so we can identify weak spots and track improvements over time. If/when we extend our module to include post run visualizations, such as a post session dashboard, Timer becomes one of the inputs to that session summary. (start code walkthrough (SCRIPT TBD))

