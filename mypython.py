# Name: Jacob Wilson
# Course: CS344_400
# Assignment: Program 5 Python Exploration
# Due Date: 11/26/16
# Desription: A simple Python program creates three
#  files containing 10 random lower-case characters
#  and displys the contents of these files. Also
#  chooses two random integers and multiplies them
#  together.

import string
import random

# FIRST PART:
print "\n"
print "\to------------o"
print "\t|  Part One  |"
print "\to------------o"
print "- create three new files"
print "- display contents of each file\n"

# create a list of three filenames to choose from
file_names = ['first_file.txt', 'second_file.txt', 'third_file.txt']

# varaibles to track current file name and file number
curr_name = ""
file_num = 0

print "contents of each file:"

# outer loop is used to create the three files
for num in range(0,3):
   # choose a unique name, create variable to hold new string
   curr_name = file_names[file_num]
   new_string = ""
   # create a new file using the current filename
   file_obj = open(curr_name, "w")

   # inner loop creates 10 character string
   for num in range(0,10):
      # choose a single character randomly and add to string
      new_char = random.choice(string.ascii_lowercase)
      new_string += new_char

   # add newline character, write string to file, and close file
   new_string += "\n"
   file_obj.write(new_string)
   file_obj.close

   # print new string and move to next filename
   print "%s: %s" % (curr_name, new_string)
   file_num += 1

# SECOND PART:
print "\n"
print "\to------------o"
print "\t|  Part Two  |"
print "\to------------o"
print "- multiply two random integers\n"

# create two random integers between 1 and 42
first_rand = random.randint(1,42)
second_rand = random.randint(1,42)

# display the random numbers
print "first random integer: %d" % first_rand
print "second random integer: %d" % second_rand

# multiply them together
product = first_rand * second_rand

# display the result
print "the product is: %d * %d = %d" % (first_rand, second_rand, product)
print "\n"
