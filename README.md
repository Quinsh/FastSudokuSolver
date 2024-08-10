
# Sudoku Solver: Crook's Algorithm Implementation.

Sudoku is a famous NP Complete problem.

I previously solved it with `ordinary backtracking`, but I was curious what other algorithms, and I felt like   
_~~I needed some intellectual challenge these days~~_. Thus, I decided to implement different algorithms and compare.

Based on some research, `Crook's Algorithm` seemed fun and intuitive and I didn't find implementation of it online,  
so decided to stick with it. Crook's Algorithm is basically **`Heuristics + Backtracking`**. We find the "humanly possible"  
solutions with Crook's heuristics, and then finish the remaining cells with backtracking (a.k.a "random guessing", he says).

I also saw some Neurodynamic Algorithms based on discrete Hopfield Networks or Boltzmann Machines. But  
I'm not confident doing ML in C++, therefore, I'm not implementing those.

Just to have some visual interface, I used OpenCV as well.

---

**Hours Spent: `22 hours`  
Lines of Code: `671 lines`**
