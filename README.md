# Context_adaptive_huffman_coding

- Explanation
  Create a general Huffman code table and a context adaptive Huffman table from the training input.
  Encode and decode the test file using the table.
  Even if it is a character that does not come from the training input, it must be matched for all characters because it can come from the test input.
  To create a context adaptive Huffman table, the frequency of each character was measured, and a Huffman table was created for each previous character.
  The adaptive Huffman table was used when the frequency of the previous character was greater than or equal to a predetermined value.
  
  
- process
  1. txt file to find the number of occurrences of the current character and the previous character.
  2. Create an adaptive Huffman table.
    2-1. Subtract the two least probable characters and put them in left and right.
    2-2. Repeat operation 2-1 until there is 1 left in the multimap.
    2-3. Put 0 in the left and 1 in the right and find the codeword.
  3. Encode the table.
    3-1. If the frequency of the previous character is more than the standard, a table is created.
    3-2. For each previous character, put eof at the end of the talbe.
  4. encode the code.
  5. Decode the context_adaptive_huffman_table.hbs file.
  6. Decode the code.hbs file.
  7. Output the resulting txt file.

  
 - Results and validation
  You can check the compression rate for each test file and check whether the result file is the same as the input file.
