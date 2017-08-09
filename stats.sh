#!/bin/bash
# Name: Jacob Wilson
# ONID username/ID: wilsjaco/932765683
# Due Date: 10/11/16
# Description: Program 1 is a bash shell script that calculates
#  the average and median of values provided in a test file. The
#  user can perform the calculations on rows or columns according
#  to the usage: stats {-rows | -cols} [file].
#
# References: http://stackoverflow.com/questions/2395284/round-
#             a-divided-number-in-bash

# FIRST BLOCK: VALIDATE INPUT
# are number of arguments correct (between 1 and 2)?
if [ "$#" -gt 2 ] || [ "$#" -lt 1 ]
then
   # error if arguments are >2 or <1
   echo "Usage: stats {-rows | -cols} [file]" 1>&2
   exit 1
fi

# can the file be read?
if [ ! -r $2 ]
then
   # error if unreadable
   echo "Cannot read file." 1>&2
   exit 1
fi

# does the file contain data?
if [ ! -s $2 ]
then
   # error if file is empty
   echo "File is empty."
   exit 0
fi

# SECOND BLOCK: SET INPUT
# FILES - utilize process ID ($$) to remain unique
input_file="input_$$"         # will store passed file
keyed_input="keyed_temp_$$"   # will store keyed-in data

# trap statement to remove the files after unexpected exit
trap "rm -rf $input_file $keyed_input; exit 1" INT HUP TERM

# if single argument present, data comes from stdin
if [ "$#" -eq 1 ]
then
  # read keyed input into a file
  while read line
  do
     echo -e "$line" >> "$keyed_input"
  done < /dev/stdin         # store into local file

   # set filed to be used to newly inputted data
  input_file="$keyed_input"
# if two arguments present, use the provided file
elif [ "$#" -eq 2 ]
then
   # set file to 2nd argument
   input_file="$2"
fi

# THIRD BLOCK: CALCULATE AND DISPLAY
# if user chooses rows
if [[ "$1" == -r* ]]
then
   echo -e "Averages Medians"
   # read in each line of the input file
   while read line
   do
      # VARIABLES for calculations
      sum=0
      avg=0
      med=0
      count=0

      # translate and sort each line
      sorted=$( echo $line | tr " " "\n" | sort -n )

      # determine average of sorted line
      for i in $sorted
      do
         sum=$(( $sum + $i ))         # accumulate the sum
         count=$(( $count + 1 ))      # count the number of entries
      done

      # prepare for rounding and calculate average (see reference)
      round_up=$(( $count / 2 ))
      avg=$(( ($sum + $round_up) / $count ))

      # determine median by cutting middle position
      middle_pos=$(( ($count / 2) + 1 ))
      med=$( echo $sorted | cut -d " " -f $middle_pos )

      # display results
      echo -e "$avg \t $med"
   done < "$input_file"
# if user chooses columns
elif [[ "$1" == -c* ]]
then
   # deterine number of columns using word count
   num_cols=$(head -n 1 $input_file | wc -w)

   # VARIABLES for loop control
   col_count=1      # used to determine cut location
   index=0          # for limiting number of loops

   # loop through the input file creating a new column with each loop
   while [ $index -lt $num_cols ]
   do
      # VARIABLES for calculations
      sum=0
      avg=0
      med=0
      count=0

      # cut, translate, and sort each input from file
      sorted=$( cut -f $col_count $input_file | tr " " "\n" | sort -n )

      # determine average of sorted column
      for i in $sorted
      do
         sum=$(( $sum + $i ))        # accumulate the sum
         count=$(( $count + 1 ))     # count the number of entries
      done

      # prepare for rounding and calculate average (see references)
      round_up=$(( $count / 2 ))
      avg=$(( ($sum + $round_up) / $count ))

      # determine median by cutting middle position
      middle_pos=$(( ($count / 2) + 1 ))
      med=$( echo $sorted | cut -d " " -f $middle_pos )   

      # store average and median in arrays
      avgArray["$index"]=$avg
      medArray["$index"]=$med

      # increment loop counters
      col_count=$(( $col_count + 1 ))
      index=$(( $index + 1 ))
   done

   # display results by looping through each array
   printf "Averages:\n"
   for i in "${avgArray[@]}"
   do
      printf '%d   ' "$i"
   done
   printf "\n"

   printf "Medians:\n"
   for i in "${medArray[@]}"
   do
      printf '%d   ' "$i"
   done
   printf "\n"

   # delete arrays
   unset avgArray
   unset medArray
# if syntax is incorrect display error and exti
else
   echo "Usage: stats {-rows | -cols} [file]" 1>&2
   exit 1
fi

# remove the file created for stdin
rm -f $keyed_input
exit 0
