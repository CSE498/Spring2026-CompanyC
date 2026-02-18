## Overview/Intro:

"Our team (23) is all things Data Analytics. We track what happens as the world runs; agent actions, world changes, key metrics, etc. Then turn we turn that data into post-run insights/ visualizations for debugging, fine tuning, and optimization.  
We support five core classes in our analytics layer: ActionLog, DataLog, Timer, ReplayDriver, and Output Manager."

## Timer (Lauren):

Timer is the part of our analytics layer that measures runtime(stopwatch sytem). It's name based, so we can time specific sections like world updates, agent decisions, or rendering steps without mixing them together. Each time you stop a timer, it records the duration and updates simple summary stats: count, last, min, max, and avg. Then later, those numbers can be reported out for performance issues or debugging. Right now, Timer primarily supports developers through performance profiling so we can identify weak spots and track improvements over time. (start code walkthrough)
we start by standardizing the time source (using Clock = std::chrono::steady_clock;
) so every measurement uses the same clock and were consistenly measuring elapsed time. The core storage is this member(std::unordered_map<std::string, TimerEntry> mTimers;). This is our registry; each timer name maps to its own stats so different sections can be timed independently. The bulk of timer is Start(name) then Stop(name). In start, this line is key(auto& entry = mTimers[name];). It grabs the entry for that name and auto creates if it's new. Then we immediately guard against misuse with the assert so starting the same timer twice (without stopping) is caught. After that we mark it running and store the start time with Clock::now(). Stop(name) is the recording step. We use find so we don't accidentally create a timer on stop, assert the name exists and is running, then compute elapsed time as now - start. Once that is converted to seconds, we update the stats in order. Set lastSeconds, add into totalSeconds, increment count, and update minSeconds and maxSeconds. The first run initializes them and later runs compare and adjust. Then we have the helpers Reset(name) and ResetAll(), so you can clear one timer's history or wipe everything. And after that, the rest of the public methods are basically the "reporting" fns. HasData, Last, Min, Max, Average, and Count. The functionality of these is exactly what it sounds like, and they are written to be safe: if a timer doesn't exist yet or has no completed runs, they just return zeros. Finally, the Catch2 tests confirm that exact behavior: empty timers return zeros, single and multiple runs update stats correctly, names stay independent and Reset/ResetAll clear everything back out. 

## ReplayDriver (Meghan):

This class implements a replay system that takes a previously recorded ActionLog and plays those actions back into the world exactly as they originally happened. The goal is to reproduce agent behavior deterministically without requiring player input. The key idea is that instead of recomputing decisions, the replay driver stores a timeline of events.
Each event contains two things: which agent performed the action and what action they performed. The driver maintains a list of these events and a pointer to where we currently are in the replay. So the system behaves similar to a tape player where it just advances forward and feeds actions back into the world. When replay starts, it convert the ActionLog into a flat sequence of events. The log is organized per agent, but replay needs to be chronological, so it extract each recorded action and store it in order. Once that list exists, the simulation no longer depends on AI or player input, it just consumes the recorded actions. During update, the driver checks if replay is running and not paused. If so, it sends the next recorded action into the world. To do that, it translates the stored action name into the agent’s action ID, and then call the world’s action function. The world executes the move exactly the same way as during gameplay, which guarantees the replay behaves identically to the original run. So the replay driver doesn’t simulate logic, it re-feeds decisions back into the existing game systems. I also implemented playback controls. Pause stops advancing the event pointer, resume continues it, and reset moves the replay back to the beginning. The driver tracks whether it’s running, paused, or finished so the rest of the program can react appropriately. The main design idea is separation of responsibility where
the world still handles gameplay rules, agents still define actions, and the replay driver only schedules and injects recorded inputs. And that’s the replay driver.



## OutputManager (Ismail):
The OutputManager is our centralized logging system in the Data Analytics layer. While the other classes track events, timings, or replay data, OutputManager is responsible for reporting and formatting information so developers can understand what the system is doing while it runs.

Its main job is to provide a single, consistent way for any part of the project — agents, worlds, replay systems, or analytics tools — to log messages. Instead of each module printing directly to the console, they call OutputManager, which handles filtering, formatting, and routing the output.

One of the core features is log level filtering. The class supports levels like Normal, Verbose, and Debug. The current level determines which messages are shown. For example, in Normal mode only important system messages appear, while Debug mode allows detailed runtime information useful for troubleshooting or testing.

Another key feature is structured metadata support. Each log message can optionally include a tag, agent ID, or simulation tick. This makes it easier to trace events in large simulations where many agents and systems are running at once. The formatting logic ensures this information appears consistently, which helps when reviewing logs or debugging replay behavior.

OutputManager also supports multiple output targets. Messages can be written to the console for real-time debugging, saved to a file for later inspection, or stored in an internal memory buffer. The buffer is especially useful for analytics or testing, since other parts of the system can retrieve those messages programmatically.

From a design standpoint, OutputManager separates logging from application logic. Modules don’t need to worry about timestamps, formatting, or where messages go — they simply call the logging function and pass a message and optional context. This keeps the codebase cleaner and ensures that all system output is handled consistently.

So overall, OutputManager acts as the communication layer for runtime information, making debugging easier, improving traceability of agent actions, and supporting analysis of simulation behavior as the project scales.

## ActionLog (Collin):

I’m going to walk through the  ActionLog class, which is designed to keep a record of all actions performed by agents in a world 
First, let’s look at the includes. We have AgentBase.hpp for the agent base class, plus chrono for time measurement, memory for smart pointers, string, unordered_map, and vector. 
Before the class, there’s a simple struct called ActionEntry. It holds 2 pieces of information:
timeOfAction, a high‑resolution time point captured when the action occurs.
actionType, a string describing what the agent did.
Now, inside the ActionLog class, the core data member is an unordered_map. Its key is a shared_ptr<AgentBase>, and its value is a vector of ActionEntry objects. This means for each agent, we store a list of all the actions that agent has performed. Using a shared_ptr as the key which allows the map to automatically manage the lifetime of the agent pointer
There’s also a simulationStartTime member, initialized in the constructor to the moment the ActionLog object is created. 
Let’s look at the methods. The most important one is recordAction. It takes an agent pointer and an action string. First, it checks if the agent pointer is valid – if it’s null, we just return. Then it captures the current time using high_resolution_clock::now(). It creates an ActionEntry, fills in the time and action type, then pushes it into the vector for that agent. The map’s operator[] will either find the existing entry for that agent or create a new one, so we don’t have to worry about whether we’ve seen this agent before.
The getActions method returns a constant reference to the entire map – useful if you want to iterate over all agents and their logs, such as in the ReplayDriver class. And getActionsByAgent lets you retrieve just the vector for a specific agent. If the agent isn’t found, it returns an empty vector.
The clear method simply empties the entire map, discarding all logs.

## DataLog (Muhammad):

Hello everyone, my name is Muhammad, and I worked on the Datalog class. This class tracks a series of data values over time and can return useful statistics such as the mean, median, minimum, and maximum. It has four private member variables: mDataValues, which stores each data point in a vector of doubles, along with mSum, mMin, and mMax. mSum keeps a running total of all values added so far, while mMin and mMax keep track of the current minimum and maximum values.
The class includes several functions. The add function inserts a new data value into the log. The mean function returns the average of the values. The median function returns the median. The minimum and maximum functions return the smallest and largest values, respectively. The count function returns the number of values currently stored, isEmpty returns true if the log is empty and false otherwise, and clear removes all values from the log.
Now, going through the implementation, the add function first checks whether the value is valid. If it is, it adds the value to the mDataValues vector and then updates mSum, mMin, and mMax. The mean function returns the average by dividing the sum by the number of values currently stored. For the median, it’s important to note how the function handles different dataset sizes. If there is an odd number of values, it returns the middle value. If there is an even number of values, it returns the average of the two middle values.
Finally, minimum returns the minimum value, maximum returns the maximum value, count returns the size of the vector, isEmpty checks whether the vector is empty, and clear resets the log by removing all stored values. That’s the overview of the Datalog class. If anyone thinks additional functionality or other statistics would be useful, feel free to let me know.

