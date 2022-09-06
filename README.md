# Sounds

Sounds library with C++, liboggvorbis and openAL

## Development

Build environment setup:

    git clone --recurse-submodules ...
    cd sounds
    mkdir build && cd build
    conan install ../conan --build=missing
    cmake ../
    # Actual build command
    cmake --build ./

## Notes

### How does markov junior works ?
* From a model file we create an interpreter
    * an interpreter contains a grid that holds the state of the current space
    * an interpreter contains the possible values taken by the input and output of the different rules
    * an interpreter contains the root (the top node) of the model and the current node that needs to be run
    * it also contains the seed for the generation
    * running the interpreter for `i` steps and with the `s` seed is running the current node `i` time or until the current node is nulled

### What is a node ?
* A node can be a branch or a rule node
* A branch is either a sequence or a markov chain
    * a sequence is a branch for which all the children are run sequentially (after each success the next node to be ran is the next node)
    * a markov chain is a branch for which all the children are tentatively run each time it's ran (after each success the next node to be ran is the first node)
* A rule node is a collection of rules

### What is a rule ?
* A rule is a transformation done from input to output
