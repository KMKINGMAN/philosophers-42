# Philosophers Project - Evaluation Questions

## 🎯 General Project Understanding

### Basic Concept Questions
1. **What is the Dining Philosophers problem and why is it important in computer science?**
   - Explain the classical scenario with philosophers, forks, and eating
   - Why is it used to demonstrate concurrency issues?
   - What real-world scenarios does it represent?

   **Answer:**
   The Dining Philosophers problem involves N philosophers sitting around a circular table with one fork between each pair of adjacent philosophers. Each philosopher alternates between thinking, eating, and sleeping. To eat, a philosopher needs both their left and right forks.

   **Why it's important:**
   - **Deadlock demonstration**: If all philosophers pick up their left fork simultaneously, deadlock occurs
   - **Resource contention**: Shows how processes compete for limited shared resources
   - **Synchronization challenges**: Illustrates the complexity of coordinating concurrent processes

   **Real-world scenarios:**
   - Database transactions accessing shared tables
   - Process scheduling in operating systems
   - Network resource allocation
   - Multi-threaded applications accessing shared data structures

2. **What are the main challenges this problem illustrates?**
   - Deadlock
   - Race conditions
   - Starvation
   - Resource contention

   **Answer:**
   - **Deadlock**: When all philosophers hold one fork and wait for another, creating a circular dependency where no one can proceed
   - **Race conditions**: Multiple threads accessing shared resources (forks) simultaneously without proper synchronization
   - **Starvation**: Some philosophers might never get to eat if others monopolize the forks
   - **Resource contention**: Limited forks (N forks for N philosophers) create competition for shared resources

3. **What happens if all philosophers simultaneously pick up their left fork?**
   - Explain the deadlock scenario
   - Why can't any philosopher proceed to eat?

   **Answer:**
   This creates a classic deadlock scenario:
   - Each philosopher holds their left fork
   - Each philosopher waits for their right fork (which is held by their neighbor)
   - No philosopher can release their fork because they haven't eaten yet
   - No philosopher can acquire their second fork because their neighbor won't release it
   - The system enters an infinite wait state where no progress is possible

   This is why deadlock prevention strategies (like the even/odd fork ordering) are essential.

### Program Arguments
4. **Explain each program argument and its purpose:**
   ```bash
   ./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
   ```
   - What units are the time values in?
   - What happens if the optional 5th argument is not provided?
   - What are valid ranges for each argument?

   **Answer:**
   - **number_of_philosophers**: Number of philosophers (and forks) at the table. Must be ≥ 1
   - **time_to_die**: Maximum time (in milliseconds) a philosopher can go without eating before dying. Must be > 0
   - **time_to_eat**: Time (in milliseconds) it takes for a philosopher to eat. Must be > 0
   - **time_to_sleep**: Time (in milliseconds) a philosopher sleeps after eating. Must be > 0
   - **[number_of_times_each_philosopher_must_eat]**: Optional. If provided, simulation stops when all philosophers have eaten this many times. Must be > 0

   **Time units**: All time values are in milliseconds
   **Without 5th argument**: Simulation continues until a philosopher dies
   **Valid ranges**: All arguments must be positive integers within reasonable limits (typically 1-200 philosophers, reasonable timing values)

5. **What should happen with invalid arguments?**
   - Negative numbers
   - Zero values
   - Non-numeric input
   - Wrong number of arguments

   **Answer:**
   The program should handle invalid input gracefully:
   - **Negative numbers**: Error message and exit with failure status
   - **Zero values**: Error message and exit (all parameters must be positive)
   - **Non-numeric input**: `ft_atoi` should handle gracefully, but program should validate and reject
   - **Wrong number of arguments**: Must be exactly 4 or 5 arguments, otherwise show usage and exit

   Example error handling:
   ```c
   if (argc < 5 || argc > 6)
       return (printf("Error: Invalid number of arguments\n"), FAILURE);
   if (data->num_philosophers <= 0 || data->time_to_die <= 0)
       return (printf("Error: Invalid arguments\n"), FAILURE);
   ```

## 🧵 Threading and Concurrency

### Thread Management
6. **How many threads does your program create?**
   - One thread per philosopher + monitor thread
   - Explain the role of each thread type

   **Answer:**
   Total threads = N + 1 (where N = number of philosophers)

   **Philosopher threads** (N threads):
   - Each executes the philosopher routine (eat-sleep-think cycle)
   - Manages their own state and fork acquisition
   - Runs until death, meal completion, or simulation stop

   **Monitor thread** (1 thread):
   - Continuously checks for philosopher deaths
   - Monitors meal completion (if 5th argument provided)
   - Stops simulation when termination conditions are met
   - Runs independently from philosopher threads

7. **Why do you need a monitor thread?**
   - What does it monitor?
   - How often does it check?
   - What happens when it detects a death?

   **Answer:**
   **Purpose of monitor thread:**
   - Detects when philosophers die (exceed time_to_die without eating)
   - Checks if all philosophers have eaten enough times (when 5th argument provided)
   - Provides centralized termination control

   **What it monitors:**
   - Each philosopher's last_meal_time vs current time
   - Each philosopher's eating status
   - Meal count completion condition

   **Check frequency:**
   - Typically every 1000 microseconds (`usleep(1000)`)
   - Frequent enough to detect deaths promptly but not too CPU-intensive

   **When death detected:**
   - Sets `simulation_stop = 1`
   - Prints death message with timestamp and philosopher ID
   - Terminates monitor thread (which signals end of simulation)

8. **How do you ensure all threads start at approximately the same time?**
   - Explain the `all_threads_ready` flag mechanism
   - Why is synchronization important here?

   **Answer:**
   **Synchronization mechanism:**
   ```c
   // 1. Create all threads first (they wait)
   // 2. Set all_threads_ready = 1
   // 3. All threads proceed simultaneously

   void wait_for_all_threads(t_philo *philo) {
       pthread_mutex_lock(&philo->data->state_mutex);
       while (!philo->data->all_threads_ready) {
           pthread_mutex_unlock(&philo->data->state_mutex);
           usleep(100);
           pthread_mutex_lock(&philo->data->state_mutex);
       }
       pthread_mutex_unlock(&philo->data->state_mutex);
   }
   ```

   **Why synchronization matters:**
   - Prevents early philosophers from monopolizing forks
   - Ensures fair initial conditions
   - Makes timing more predictable and testing more reliable
   - Simulates real-world scenario where all philosophers arrive simultaneously

9. **What is the purpose of staggering even-numbered philosophers?**
   ```c
   if (philo->id % 2 == 0)
       usleep(1000);
   ```
   - How does this help prevent deadlock?

   **Answer:**
   **Purpose of staggering:**
   - Breaks initial synchronization that could lead to immediate deadlock
   - Creates temporal separation between even and odd philosophers
   - Allows odd philosophers to start first, creating an initial advantage

   **How it prevents deadlock:**
   - When odd philosophers (1, 3, 5...) start first, they grab their left forks
   - Even philosophers (2, 4, 6...) start slightly later when some forks are already taken
   - This prevents the "all grab left fork simultaneously" scenario
   - Creates a staggered acquisition pattern that naturally breaks circular waiting

   **Combined with fork ordering:** This staggering works together with the even/odd fork ordering strategy to provide robust deadlock prevention.

### Mutexes and Synchronization
10. **How many mutexes does your program use and what is each one for?**
    - Fork mutexes (one per fork)
    - Print mutex
    - State mutex
    - Meal mutexes (one per philosopher)

    **Answer:**
    Total mutexes = 2N + 2 (where N = number of philosophers)

    **Fork mutexes** (N mutexes):
    - One per fork to ensure exclusive access
    - Prevents race conditions when acquiring/releasing forks
    - Critical for preventing multiple philosophers from using same fork

    **Print mutex** (1 mutex):
    - Ensures atomic printing of status messages
    - Prevents output from different threads from mixing/interleaving
    - Maintains clean, readable output format

    **State mutex** (1 mutex):
    - Protects shared simulation state (`simulation_stop`, `all_threads_ready`)
    - Ensures consistent view of global simulation status
    - Coordinates between monitor and philosopher threads

    **Meal mutexes** (N mutexes):
    - One per philosopher to protect meal-related data
    - Protects `last_meal_time`, `eating` flag, `meals_eaten` counter
    - Prevents race conditions during meal status updates

11. **Why do you need a print mutex?**
    - What happens without it?
    - Demonstrate with an example

    **Answer:**
    **Without print mutex**, output from multiple threads can interleave:
    ```
    // BAD - Mixed output:
    10 1 has taken 15 2 has taken a fork
    a fork
    25 1 is eating
    30 2 is eat35 3 has taken a fork
    ing
    ```

    **With print mutex**, output is atomic:
    ```
    // GOOD - Clean output:
    10 1 has taken a fork
    15 2 has taken a fork
    25 1 is eating
    30 2 is eating
    35 3 has taken a fork
    ```

    **Implementation:**
    ```c
    void print_status(t_philo *philo, char *status) {
        pthread_mutex_lock(&philo->data->print_mutex);
        if (!philo->data->simulation_stop) {
            printf("%lld %d %s\n", timestamp, philo->id, status);
        }
        pthread_mutex_unlock(&philo->data->print_mutex);
    }
    ```

12. **Explain the purpose of the state mutex:**
    - What shared variables does it protect?
    - When is it locked/unlocked?

13. **What is protected by each philosopher's meal mutex?**
    - `last_meal_time`
    - `eating` flag
    - `meals_eaten` counter

## 🍴 Fork Management and Deadlock Prevention

### Fork Assignment Strategy
14. **How do you prevent deadlock in your solution?**
    - Explain the even/odd fork ordering strategy:
    ```c
    if (philo->id % 2 == 0) {
        *first = philo->right_fork;
        *second = philo->left_fork;
    } else {
        *first = philo->left_fork;
        *second = philo->right_fork;
    }
    ```

    **Answer:**
    **Even/Odd Fork Ordering Strategy:**
    - **Odd philosophers** (1, 3, 5...): Pick up LEFT fork first, then RIGHT fork
    - **Even philosophers** (2, 4, 6...): Pick up RIGHT fork first, then LEFT fork

    **How it works:**
    1. Breaks the symmetry that causes deadlock
    2. Ensures at least one philosopher can acquire both forks
    3. Creates a partial ordering of resource acquisition
    4. Prevents circular wait condition

    **Example with 5 philosophers:**
    - P1: left(1) → right(2)
    - P2: right(2) → left(2) ← CONFLICT with P1, but P1 gets precedence
    - P3: left(3) → right(4)
    - P4: right(4) → left(4) ← CONFLICT with P3, but P3 gets precedence
    - P5: left(5) → right(1) ← Can't get right(1) if P1 has it

    This asymmetric approach guarantees that deadlock cannot occur.

15. **Why does the fork ordering prevent deadlock?**
    - Explain how it breaks the circular wait condition
    - What would happen if all philosophers picked up forks in the same order?

    **Answer:**
    **Breaking circular wait condition:**
    Deadlock requires 4 conditions (Coffman conditions):
    1. Mutual exclusion ✓ (forks can't be shared)
    2. Hold and wait ✓ (philosophers hold one fork while waiting for another)
    3. No preemption ✓ (forks can't be forcibly taken)
    4. **Circular wait** ← This is what we break!

    **How even/odd ordering breaks circular wait:**
    - In a circular wait, each philosopher waits for the next philosopher's resource
    - With even/odd ordering, we create a "hierarchy" where some philosophers get priority
    - At least one philosopher in the circle will be able to complete their acquisition
    - This breaks the circular dependency chain

    **If all philosophers used same order:**
    ```c
    // If everyone went left-then-right:
    P1: gets fork1, waits for fork2
    P2: gets fork2, waits for fork3
    P3: gets fork3, waits for fork4
    P4: gets fork4, waits for fork5
    P5: gets fork5, waits for fork1 ← CIRCULAR WAIT = DEADLOCK
    ```

    The asymmetric ordering prevents this circular dependency.

16. **How are forks assigned to philosophers?**
    ```c
    data->philosophers[i].left_fork = &data->forks[i];
    data->philosophers[i].right_fork = &data->forks[(i + 1) % data->num_philosophers];
    ```
    - Why use modulo operation?
    - Draw a diagram showing the circular arrangement

    **Answer:**
    **Fork assignment logic:**
    - **Left fork**: `&data->forks[i]` - philosopher i gets fork i as their left fork
    - **Right fork**: `&data->forks[(i + 1) % num_philosophers]` - philosopher i gets fork (i+1) as their right fork

    **Why modulo operation:**
    - Creates circular arrangement where last philosopher's right fork is first philosopher's left fork
    - For philosopher N-1: right_fork = &data->forks[(N-1 + 1) % N] = &data->forks[0]
    - This completes the circle

    **Circular arrangement diagram:**
    ```
         P0 ---- F0 ---- P1
         |               |
        F4               F1
         |               |
         P4 ---- F3 ---- P2
              F2
    ```
    - P0's forks: left=F4, right=F0
    - P1's forks: left=F0, right=F1
    - P2's forks: left=F1, right=F2
    - P3's forks: left=F2, right=F3
    - P4's forks: left=F3, right=F4 (completes circle)

### Special Cases
17. **What happens with only one philosopher?**
    ```c
    if (philo->data->num_philosophers == 1) {
        pthread_mutex_unlock(&first->mutex);
        precise_sleep(philo->data->time_to_die);
        return (1);
    }
    ```
    - Why can't a single philosopher eat?
    - How does your program handle this case?

    **Answer:**
    **Why single philosopher can't eat:**
    - A philosopher needs TWO forks to eat
    - With only one philosopher, there's only ONE fork
    - Philosopher picks up their left fork (which is also their right fork)
    - They can never acquire a second fork because it's the same fork
    - Mathematically impossible to eat with one fork

    **How the program handles it:**
    1. Philosopher picks up their only fork
    2. Realizes they can't get a second fork (single philosopher check)
    3. Immediately releases the fork
    4. Sleeps for `time_to_die` duration
    5. Dies as expected (simulation ends)

    **Alternative approaches:**
    - Some implementations might have the single philosopher die immediately
    - Others might have them wait exactly `time_to_die` milliseconds before dying
    - The key is that a single philosopher should always die since they cannot eat

18. **How do you handle the case where a philosopher dies while holding forks?**
    - Are the forks properly released?
    - Does the simulation stop immediately?

## ⏰ Timing and Death Detection

### Time Management
19. **How do you track when a philosopher last ate?**
    - When is `last_meal_time` updated?
    - Why use mutex protection for this variable?

20. **How does the death detection work?**
    ```c
    if (!philos[i].eating && (current_time - philos[i].last_meal_time) >= data->time_to_die)
    ```
    - Why check `!philos[i].eating`?
    - What happens when a death is detected?

21. **Why use `precise_sleep()` instead of regular `sleep()`?**
    - How is it implemented?
    - What are the advantages?

22. **How do you handle timing precision?**
    - Do you use milliseconds consistently?
    - How do you get current time?

## 🔄 Philosopher Lifecycle

### State Transitions
23. **Describe the lifecycle of a philosopher:**
    - Think → Hungry → Eating → Sleeping → repeat
    - How long does each state last?

24. **What happens during each action?**
    - `philo_eat()`: Acquire forks, eat, release forks
    - `philo_sleep()`: Sleep for specified time
    - `philo_think()`: Brief thinking period

25. **How do you ensure a philosopher doesn't starve?**
    - Monitor thread checks regularly
    - Fair fork acquisition strategy

### Meal Counting
26. **How do you track meals eaten by each philosopher?**
    - When is the counter incremented?
    - Is it thread-safe?

27. **What happens when all philosophers have eaten enough times?**
    - How does the monitor detect this condition?
    - How does the simulation end?

## 🐛 Error Handling and Edge Cases

### Memory Management
28. **How do you handle memory allocation failures?**
    - Fork allocation
    - Philosopher allocation
    - Cleanup on failure

29. **How do you clean up resources?**
    ```c
    void free_data(t_data *data)
    ```
    - What needs to be freed?
    - When is cleanup performed?

### Thread Safety
30. **How do you ensure no data races?**
    - Identify all shared variables
    - Show how each is protected

31. **What happens if thread creation fails?**
    - How do you handle `pthread_create()` failures?
    - Do you clean up partial thread creation?

32. **How do you handle simulation termination?**
    - Setting `simulation_stop` flag
    - Ensuring all threads exit cleanly

## 🧪 Testing and Debugging

### Test Cases
33. **What test cases should be run to verify your solution?**
    ```bash
    ./philo 1 800 200 200      # Single philosopher
    ./philo 5 800 200 200      # Basic case
    ./philo 5 800 200 200 7    # With meal count
    ./philo 4 410 200 200      # Tight timing
    ./philo 100 800 200 200    # Many philosophers
    ```

    **Answer:**
    **Essential test cases:**

    1. **Single philosopher** (`./philo 1 800 200 200`):
       - Should die after 800ms (can't eat with one fork)
       - Tests edge case handling

    2. **Basic functionality** (`./philo 5 800 200 200`):
       - No philosophers should die
       - Clean output format
       - No deadlocks

    3. **Meal count termination** (`./philo 5 800 200 200 7`):
       - Should stop when all philosophers eat 7 times
       - No death messages

    4. **Tight timing** (`./philo 4 410 200 200`):
       - Tests edge case where time_to_die is close to time_to_eat
       - Should still work without deaths

    5. **Stress test** (`./philo 100 800 200 200`):
       - Tests scalability and performance
       - Should handle many threads without issues

    6. **Invalid arguments**:
       - `./philo 0 800 200 200` (should fail)
       - `./philo 5 -1 200 200` (should fail)
       - `./philo` (wrong arg count, should fail)

34. **How do you test for deadlocks?**
    - What indicators show a deadlock?
    - How long should you wait before concluding deadlock?

    **Answer:**
    **Deadlock indicators:**
    - **No progress**: All philosophers stop eating/sleeping/thinking
    - **Frozen output**: No new status messages appear
    - **Partial fork acquisition**: Some philosophers have taken forks but none can eat
    - **No simulation termination**: Program runs indefinitely without death or completion

    **Testing methodology:**
    1. **Visual inspection**: Watch output for 10-15 seconds
    2. **Progress monitoring**: Count status messages - they should continue flowing
    3. **Specific scenarios**: Test with conditions that historically cause deadlock
    4. **Timeout approach**: If no progress for 10+ seconds, likely deadlocked

    **Time to wait:**
    - **Normal cases**: 5-10 seconds should show clear progress
    - **Large N**: For 100+ philosophers, wait up to 30 seconds
    - **Tight timing**: Cases like `./philo 4 410 200 200` may need longer observation

    **Good signs (no deadlock):**
    - Continuous stream of eating/sleeping/thinking messages
    - Natural deaths when expected
    - Proper termination on meal completion

35. **How do you verify no philosopher starves?**
    - Monitor output for death messages
    - Check meal distribution fairness

    **Answer:**
    **Starvation verification methods:**

    1. **Death message monitoring**:
       - Watch for any `"X died"` messages
       - If philosophers are eating regularly, no deaths should occur
       - Deaths indicate starvation (time_to_die exceeded)

    2. **Meal distribution analysis**:
       - Count eating messages per philosopher
       - Should be relatively balanced (no huge disparities)
       - Example: P1:5 meals, P2:6 meals, P3:4 meals ← Good balance
       - Bad: P1:20 meals, P2:1 meal, P3:0 meals ← P2 and P3 starving

    3. **Time between meals**:
       - Monitor timestamps of each philosopher's eating events
       - Time between meals should be < time_to_die
       - Large gaps indicate potential starvation risk

    4. **Fair scheduling verification**:
       - Each philosopher should get roughly equal opportunities
       - No philosopher should be consistently blocked by neighbors

    **Tools for verification:**
    ```bash
    # Count meals per philosopher
    ./philo 5 800 200 200 | grep "is eating" | cut -d' ' -f2 | sort | uniq -c

    # Monitor for deaths
    ./philo 5 800 200 200 | grep "died"
    ```

### Code-Specific Questions
36. **Explain this code snippet:**
    ```c
    pthread_mutex_lock(&first_fork->mutex);
    if (check_simulation_stop(philo)) {
        pthread_mutex_unlock(&first_fork->mutex);
        return (FAILURE);
    }
    ```
    - Why check simulation stop after acquiring first fork?
    - Why unlock before returning?

    **Answer:**
    **Why check simulation stop after acquiring fork:**
    - Simulation might have ended (death/completion) while waiting for fork
    - Philosopher could have been blocked waiting for mutex
    - Without this check, philosopher might continue trying to eat after simulation ended
    - Prevents unnecessary actions and ensures clean termination

    **Why unlock before returning:**
    - **Resource leak prevention**: If we don't unlock, fork remains locked forever
    - **Deadlock prevention**: Other philosophers waiting for this fork would block indefinitely
    - **Clean shutdown**: Ensures all resources are properly released
    - **Good practice**: Always unlock what you lock, even on error paths

    **What happens without unlock:**
    ```c
    // BAD - Fork remains locked
    pthread_mutex_lock(&first_fork->mutex);
    if (simulation_stop)
        return (FAILURE);  // Fork still locked!

    // GOOD - Proper cleanup
    pthread_mutex_lock(&first_fork->mutex);
    if (simulation_stop) {
        pthread_mutex_unlock(&first_fork->mutex);
        return (FAILURE);
    }
    ```

    This pattern ensures robust error handling and resource management.
    - Why unlock before returning?

37. **What's the purpose of this check in the eating function?**
    ```c
    if (interruptible_sleep(philo, philo->data->time_to_eat) == FAILURE) {
        update_meal_status(philo, 0);
        pthread_mutex_unlock(&second_fork->mutex);
        pthread_mutex_unlock(&first_fork->mutex);
        return (FAILURE);
    }
    ```

38. **Explain the monitor's main loop:**
    ```c
    while (1) {
        if (data->must_eat_count != -1) {
            if (check_if_all_ate(data, philos)) {
                // Stop simulation
                return (NULL);
            }
        }
        if (check_all_philos(data, philos))
            return (NULL);
        usleep(1000);
    }
    ```
    - Why check meal completion first?
    - Why the `usleep(1000)`?

## 🎮 Interactive Demonstration

### Live Testing
39. **Can you demonstrate your program with different parameters?**
    - Show normal execution
    - Show a case where a philosopher dies
    - Show meal completion scenario

40. **Can you trace through the execution of a specific philosopher?**
    - Show state transitions in the output
    - Explain the timing

### Code Walkthrough
41. **Walk through the initialization process:**
    - Data structure setup
    - Fork initialization
    - Philosopher initialization
    - Thread creation

42. **Trace a philosopher's first meal:**
    - Fork acquisition
    - Eating process
    - Fork release

## 🔧 Alternative Solutions and Improvements

### Design Decisions
43. **Why did you choose this deadlock prevention strategy over others?**
    - Compare with other approaches (resource hierarchy, arbitrator, etc.)
    - What are the trade-offs?

44. **How could you improve the solution?**
    - Better fairness
    - Reduced contention
    - More efficient synchronization

45. **What happens if you change the fork acquisition order?**
    - Always left first, then right
    - Resource hierarchy (lowest ID first)

### Performance Considerations
46. **How does your solution scale with the number of philosophers?**
    - Performance with 2 vs 100 philosophers
    - Memory usage considerations

47. **What's the bottleneck in your implementation?**
    - Mutex contention
    - Monitor thread polling frequency

## 🎯 Advanced Understanding

### Concurrency Concepts
48. **Explain the difference between deadlock and livelock:**
    - Could your solution suffer from livelock?
    - How would you detect it?

49. **What is the ABA problem and could it affect your solution?**
    - Explain with philosopher state examples

50. **How does your solution relate to other synchronization problems?**
    - Readers-writers problem
    - Producer-consumer problem
    - Sleeping barber problem

---

## 💡 Bonus Questions

### Theoretical Extensions
51. **How would you modify the solution for unequal eating times?**
52. **What if philosophers had preferences for certain forks?**
53. **How would you implement priority philosophers?**
54. **Could you solve this with semaphores instead of mutexes?**
55. **How would you visualize the current state of all philosophers?**

### Real-World Applications
56. **Give examples of real systems where similar synchronization is needed:**
    - Database transaction management
    - Process scheduling
    - Resource allocation in OS

---

## 📝 Quick Reference - Key Code Snippets to Understand

### Fork Setup Strategy
```c
void setup_forks(t_philo *philo, t_fork **first, t_fork **second) {
    if (philo->id % 2 == 0) {
        *first = philo->right_fork;
        *second = philo->left_fork;
    } else {
        *first = philo->left_fork;
        *second = philo->right_fork;
    }
}
```

### Death Detection
```c
if (!philos[i].eating && (current_time - philos[i].last_meal_time) >= data->time_to_die) {
    // Philosopher has died
    data->simulation_stop = 1;
    printf("%lld %d died\n", get_time() - data->start_time, philos[i].id);
}
```

### Thread Synchronization
```c
void wait_for_all_threads(t_philo *philo) {
    pthread_mutex_lock(&philo->data->state_mutex);
    while (!philo->data->all_threads_ready) {
        pthread_mutex_unlock(&philo->data->state_mutex);
        usleep(100);
        pthread_mutex_lock(&philo->data->state_mutex);
    }
    pthread_mutex_unlock(&philo->data->state_mutex);
}
```

### Safe Status Printing
```c
void print_status(t_philo *philo, char *status) {
    pthread_mutex_lock(&philo->data->print_mutex);
    pthread_mutex_lock(&philo->data->state_mutex);
    if (!philo->data->simulation_stop) {
        printf("%lld %d %s\n", get_time() - philo->data->start_time,
               philo->id, status);
    }
    pthread_mutex_unlock(&philo->data->state_mutex);
    pthread_mutex_unlock(&philo->data->print_mutex);
}
```
