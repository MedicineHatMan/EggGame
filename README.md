# EggGame

This is an incomplete decompilation of Billy Hatcher and the Giant Egg. 

Output is about 99% matching

<img width="639" height="139" alt="image" src="https://github.com/user-attachments/assets/720b3e81-29a2-4ea4-bccf-22a94a24f73c" />


## How

We were training a local NN model for use alongside ghidra on PPC decomp. 

We used this game as a test, and then fixed up some other things by hand. 

Quality is bad, but it gets the point across. 

## LLM's were leveraged in the creation of this.
As mentioned above, we're using this in conjunction with an LLM to train an RNN on interpreting powerpc assembly. This project is just the result of that output. 

LLM's were leveraged for refinement / translation of the RNN output, and for higher level functions, such as the concept of linking (Not the target for the RNN)

<img width="1013" height="631" alt="image" src="https://github.com/user-attachments/assets/860d2cbd-d452-476a-9991-c9f96e812237" />
