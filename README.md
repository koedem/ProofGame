This program attempts to generate proof game puzzles and similar challenges.

The Surge package is taken from https://github.com/nkarve/surge and used in only slightly modified form to give a board representation and perform move generation.

Currently there is no user interaction, instead the calculation is decided on at compile time.


What does this program do?
This program attempts to find candidates for good proofgame challenges. The main approach is to find positions that are reachable in N plies and which are uniquely reachable in N + K plies. The basic algorithm generates all depth N positions and stores them in a hashtable.
Then it performs a depth N + K search and counts occurrences in that first hashtable to tell which of those positions are uniquely reachable.

Some simple optimizations consider the fact that a depth M < N + K position that is not uniquely reachable can not possibly be extended to a N + K unique position. Therefore, after reaching a position for the third time, we can abort the search of that subtree. This is achieved by a separate hashtable that counts occurrences of those inner nodes in the deep search. Note we have to continue the search after reaching a position for the second time, since otherwise resulting positions might falsely be classified as uniquely reachable, only after a the second search through will all the corresponding leaves be guaranteed to have their counter on at least 2. However, if a lower depth position is already reachable many times, that immediately "ruins" the subtree and thus we can simply increment all subtree counters by 2 right away, including the low depth hashtable, to save the second go-through. This process is helped by the iterative deepening approach that first performs lower depth searches, then remembers the properties of the inner nodes of that search.


Not yet implemented ideas:
- If the desired difference in depth is large, the low depth hashtable will be comparatively small. One can then use an unmove generator to save one or multiple plies in the deeper search by meeting in the middle.

- Any position on the path to a uniquely reachable position must be uniquely reachable itself. Therefore, if we only look at branches in the deeper search that are uniquely reachable all the way through, we reach a subset of positions which contain all the candidates for uniquely reachable positions. The only issue is that not all of them might be uniquely reachable but also from transposing paths. However, the smaller set can be used like in idea 1, or to increase cache efficiency of the full deep search later.

- If we try to lose an even number of plies, i.e. look for depth 8 positions that are also depth 6 positions, any unique D8/6 must be unreachable D7/5 (else it now has multiple options or is an extension of a shorter such position, neither of which we are interested in). Therefore, all the candidates for that D8/6 search can be generated from only the unreachable D7/5 positions by adding a ply. This gives a smaller set which again can be used as above. Note this does not work for odd ply differences as that changes who to move it is.




Other types of similar problems:
Proofgame challenges that consider the properties of the final move. E.g. "Construct a game that ends in 5.Qxe4#".
These can be generated similarly, with a target hashtable that instead contains possible final moves or other conditions, followed by the usual search that counts occurrences.