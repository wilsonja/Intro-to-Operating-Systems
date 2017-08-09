/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Name: Jacob Wilson                                *
 * Course: CS344_400                                 *
 * Due Date: 10/27/2016                              *
 * Description: The program is a text-based game     *
 *  where a player navigates through rooms in secrch *
 *  of the end room. Rooms are built randomly and    *
 *  stored in a file. The new file is used to        *
 *  access the room data.                            *
 * References: listed at bottom of program           *                                                
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

// GLOBAL STRUCTS
// a struct for storing newly created rooms
struct Room {
   char name[50];
   char type[50];
   int num_connected;
   int first_open_pos;                   // to determine position for connected room
   int is_connected[7];                  // used as bool to help maintain unique conenctions
   int counted;
   struct Room *connected_rooms[6];      // array of structs representing connected rooms
};
// a struct for storing data from the files 
struct GameRoom {
   char name[50];
   char type[50];
   char connected_rooms[6][50];          // array of strings are names of connected rooms
   int num_connected;
};

// FUNCTION PROTOTYPES
void buildDir();
void initRooms(struct Room *);
void nameRooms(struct Room *, const char **);
void typeRooms(struct Room *);
void connectRooms(struct Room *);
void writeRooms(struct Room *);
void readRoomFiles(struct GameRoom *);
int findRoom(struct GameRoom *, char *);

// MAIN FUNCTION
int main(void) {
   srand(time(NULL));
   struct Room all_rooms[7];             // all_rooms struct is for creating room data
   const char *room_names[] = {"Office", "pikachu_room", "dungeon", "Kitchen", "bathroom",
                               "charmander_room", "Closet", "Cats", "Dogs", "utility" };
   struct GameRoom game_rooms[7];        // game_rooms struct is for inputted data from files
                                         // whcih will be used in the game play

   // SET UP THE ROOM DATA
   buildDir();                           // first create a new directory
   initRooms(all_rooms);                 // initialize the room struct data
   nameRooms(all_rooms, room_names);     // randomly assign room names to rooms in the struct
   typeRooms(all_rooms);                 // randomly assign type to the rooms
   connectRooms(all_rooms);              // randomly created connections between rooms
   writeRooms(all_rooms);                // write the rooms to a file
   readRoomFiles(game_rooms);            // read the files into a new struct for the game

   // START THE GAME!
   printf("                          \n");
   printf("  WELCOME  ---------------\n");
   printf("      TO     ----  o o  --\n");
   printf("          THE    o     o -\n");
   printf("      ADVENTURE  o     o -\n");
   printf("           GAME    o o  --\n");
   printf("     ---------------------\n");
   printf("\n");

   // Variables to track the gameplay
   int i, h;                             // loop controls
   int curr_loc = 0;                     // tracks players current room
   int game_over = 0;                    // bool to determine game ending
   int game_won = 0;                     // bool to determin game success
   int num_steps = 0;                    // number of steps taken
   int room_path[100];                   // stores names of rooms visited (max 100)
   int path_count = 0;                   // loop control for path display
   int path_index = 0;                   // for accessing name to add to path

   // First, determine the room with which to begin the game
   for (i = 0; i < 7; i++) {
      if(strcmp(game_rooms[i].type, "START_ROOM") == 0) {
         curr_loc = i;
      };
   };

   // Next, run the game loop until the end room is reached
   do {
      int j, k;                          // loop controls
      int match = 0;                     // used as bool for when use input is correct
      char user_input[50];

      // Display room info to user
      printf("%s: %s\n", "CURRENT LOCATION", game_rooms[curr_loc].name);
      printf("POSSIBLE CONNECTIONS: "); 

      // loop through all room connections and display each
      for (j = 0; j < game_rooms[curr_loc].num_connected; j++) {
         if ((j + 1) < game_rooms[curr_loc].num_connected) {
            printf("%s, ", game_rooms[curr_loc].connected_rooms[j]);
         } else {
            printf("%s.\n", game_rooms[curr_loc].connected_rooms[j]);
         };
      };

      // Obtain location movement from user
      printf("WHERE TO? > ");
      fgets(user_input, 50, stdin);

      // Replace the newline with NULL character
      int last = strlen(user_input) - 1;
      if(user_input[last] == '\n') {
         user_input[last] = '\0';
      };

      // Validate user input against available room names
      for (k = 0; k < game_rooms[curr_loc].num_connected; k++) {
         if (strcmp(user_input, game_rooms[curr_loc].connected_rooms[k]) == 0) {
            curr_loc = findRoom(game_rooms, user_input);
            // if user inputs correct room, add it to the path taken
            if (curr_loc >= 0) {
               match = 1;
               num_steps++;
               room_path[path_count] = curr_loc;
               path_count++;
            };
         };
      };

      printf("\n");

      // Check if the new room is the END_ROOM which would signal game end
      if (match == 1) {
         if (strcmp(game_rooms[curr_loc].type, "END_ROOM") == 0) {
            game_over = 1;
            game_won = 1;
         };
      } else if (match == 0) {
         // if user inputted something other than available room connections
         printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
      };
   } while (game_over == 0);

   // When the user has reached the end room, display end results
   if (game_won == 1) {
      printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
      printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_steps);
      // loop through path array displaying each room visited
      for (h = 0; h < path_count; h++) {
         path_index = room_path[h];
         printf("%s\n", game_rooms[path_index].name);
      };
      printf("\n");
   };

   exit(0);
};

//FUNCTIONS

/* * * * * * * * * * * * * * * * * * * * * * * 
 * Name: buildDir()
 * Parameters: None
 * Description: The function creates a new 
 *  directory in the correct format and 
 *  changes to this new directory.
 * * * * * * * * * * * * * * * * * * * * * * */
void buildDir()
{
   char new_dir[50];

   // create a formatted string, getpic() for current process ID
   sprintf(new_dir, "%s%d", "wilsjaco.rooms.", getpid());

   // make the directory and move to it
   mkdir(new_dir, 0755);
   chdir(new_dir);
};

/* * * * * * * * * * * * * * * * * * * * * * * 
 * Name: initRooms
 * Parameters: a Room struct
 * Description: The function sets all Room
 *  struct data to initial zero state.
 * * * * * * * * * * * * * * * * * * * * * * */
void initRooms(struct Room *room_array)
{
   int i = 0;
   int j;

   // loop through all rooms setting data to zero
   while (i < 7) {
      room_array[i].first_open_pos = 0;
      room_array[i].num_connected = 0;
      room_array[i].counted = 0;

      // another loop for the connected rooms
      for (j = 0; j < 7; j++) {
         room_array[i].is_connected[j] = 0;
      };
      i++;
   };
};

/* * * * * * * * * * * * * * * * * * * * * * * 
 * Name: nameRoomed
 * Parameters: Room struct and char array
 * Description: The function chooses a random
 *  name from the arary and assigns it to
 *  a room.
 * * * * * * * * * * * * * * * * * * * * * * */
void nameRooms(struct Room *room_array, const char **room_name)
{
   int i = 0;
   int name_is_used[10] = {0};

   // make seven room name assignments
   while (i < 7) {
      int rand_index = rand() % 10;

      // check that name hasn't been used and assign
      if (!name_is_used[rand_index]) {
         strcpy(room_array[i].name, room_name[rand_index]);
         name_is_used[rand_index] = 1;
         i++;
      };
   };
};

/* * * * * * * * * * * * * * * * * * * * * * * 
 * Name: typeRooms
 * Parameters: Room struct
 * Description: The function chooses a room
 *  randomly and assigns a type.
 * * * * * * * * * * * * * * * * * * * * * * */
void typeRooms(struct Room *room_array)
{
   int i = 0;
   int type_is_assigned[7] = {0};
   int start_is_used = 0;
   int end_is_used = 0;

   // make all seven room assignments
   while (i < 7) {
      int rand_index = rand() % 7;

      // make assignments based on what has already been used
      if (!type_is_assigned[rand_index]) {
         if (!start_is_used) {
            strcpy(room_array[rand_index].type, "START_ROOM");
            type_is_assigned[rand_index] = 1;
            start_is_used = 1;
         } else if (!end_is_used) {
            strcpy(room_array[rand_index].type, "END_ROOM");
            type_is_assigned[rand_index] = 1;
            end_is_used = 1;
         } else {
            strcpy(room_array[rand_index].type, "MID_ROOM");
            type_is_assigned[rand_index] = 1;
         };
         i++;
      };
   };
};

/* * * * * * * * * * * * * * * * * * * * * * * 
 * Name: connectRooms
 * Parameters: Room struct
 * Description: The function makes random room
 *  assignments. It randomly chooses two rooms
 *  and checks for a unique connection. Then
 *  it makes reciprocal connections and tracks
 *  the connection for each room.
 * * * * * * * * * * * * * * * * * * * * * * */
void connectRooms(struct Room *room_array)
{
   int all_connected = 0;

   // make connections for all seven rooms
   while (all_connected < 7) {
      int first_rand = rand() % 7;       // for the first randomly chosen room
      int second_rand = rand() % 7;      // for the second randomly chosen room

      // check that connections don't exceed 6 and that you aren't connecting one room to itself
      if (room_array[first_rand].num_connected < 6 && room_array[second_rand].num_connected < 6 && first_rand != second_rand) {
         // make a connection if the rooms aren't already connected to each other
         if (!room_array[first_rand].is_connected[second_rand]) {
            // first make the two connections
            room_array[first_rand].connected_rooms[room_array[first_rand].first_open_pos] = &room_array[second_rand];
            room_array[second_rand].connected_rooms[room_array[second_rand].first_open_pos] = &room_array[first_rand];
            // update all variables to reflect the new connections
            room_array[first_rand].is_connected[second_rand] = 1;
            room_array[second_rand].is_connected[first_rand] = 1;
            room_array[first_rand].num_connected++;
            room_array[second_rand].num_connected++;
            room_array[first_rand].first_open_pos++;
            room_array[second_rand].first_open_pos++;

            // when both rooms have at least three connections, you are one step closer to having all connected
            if (room_array[first_rand].num_connected > 3 && room_array[first_rand].counted == 0) {
               all_connected++;
               room_array[first_rand].counted = 1;
            };
            if (room_array[second_rand].num_connected > 3 && room_array[second_rand].counted == 0) {
               all_connected++;
               room_array[second_rand].counted = 1;
            };
         };
      };
   };
   printf("\n");
};

/* * * * * * * * * * * * * * * * * * * * * * * 
 * Name: writeRooms
 * Parameters: Room struct
 * Description: The function takes data from
 *  the newly created room structs and writes
 *  them to files in the accepted format.
 * * * * * * * * * * * * * * * * * * * * * * */
void writeRooms(struct Room *room_array) {
   int i, j;
   const char *file_name;

   // loop through all rooms in the struct
   for (i = 0; i < 7; i++) {
      file_name = room_array[i].name;    // use the room name as the file name

      FILE *output_file = fopen(file_name, "w");

      if (output_file == NULL) {
         printf("Error opening file.");
         exit(1);
      };

      // populate the new file with the data
      fprintf(output_file, "ROOM NAME: %s\n", room_array[i].name);

      // loop to get through all of the connections
      for (j = 0; j < room_array[i].num_connected; j++) {
         fprintf(output_file, "CONNECTION %d: %s\n", j + 1, room_array[i].connected_rooms[j]->name);
      };

      fprintf(output_file, "ROOM TYPE: %s\n", room_array[i].type);

      fclose(output_file);
   };
};

/* * * * * * * * * * * * * * * * * * * * * * * 
 * Name: readRoomFiles
 * Parameters: GameRoom struct
 * Description: The function reads files in
 *  the current directory and populates data
 *  in the GameRoom struct.
 * * * * * * * * * * * * * * * * * * * * * * */
void readRoomFiles(struct GameRoom *game_room)
{
   DIR *dir_ptr;
   FILE *file_ptr;
   struct dirent *dir_entry;             // holds each file in the directory
   char *file_name;
   char buffer[50];                      // stores each line scanned from the file
   char target[50];                      // stores the target string in the line
   char compare[50];                     // stores either TYPE: and NAME: from line
   char connect[50];                     // used to determine if file line is a CONNECTION
   int curr_room = -2;                   // offset of two for correct counting of files
   int connect_index = 0;                // count of each connecting room line in file

   // opent the current directory
   dir_ptr = opendir("./");
   if (dir_ptr != NULL) {
      // loop while there are files in the directory
      while (dir_entry = readdir(dir_ptr)) {
         file_name = dir_entry->d_name;
         file_ptr = fopen(file_name, "r");    // open the file
         if (file_ptr == NULL) {
            printf("Could not open file");
            exit(1);
         };
         // loop while there are lines in teh file
         while (fgets(buffer, sizeof(buffer), file_ptr) != NULL) {
            // scan each line of the file and store the data in it
            sscanf(buffer, "%s %s %s\n", connect, compare, target);
            // compare parts of the line to determine the contents of each,
            // then store in the correct struct location
            if (strcmp(compare, "TYPE:") == 0) {
               strcpy(game_room[curr_room].type, target); 
            } else if (strcmp(compare, "NAME:") == 0) {
               strcpy(game_room[curr_room].name, target);
            } else if (strcmp(connect, "CONNECTION") == 0) {
               strcpy(game_room[curr_room].connected_rooms[connect_index], target);
               game_room[curr_room].num_connected = connect_index + 1;
               connect_index++;
            };
         };

         fclose(file_ptr);
         connect_index = 0;              // reset for accurate count of connection lines
         curr_room++;
      };
   } else {
      printf("Could not open the directory");
   };
};

/* * * * * * * * * * * * * * * * * * * * * * * 
 * Name: findRoom
 * Parameters: GameRoom struct and string
 * Description: The function searches for a
 *  room in the array that matches names with
 *  the passed search name. It returns the
 *  index of the room if found.
 * * * * * * * * * * * * * * * * * * * * * * */
int findRoom(struct GameRoom *room_array, char *search)
{
   int i;

   // loop through each room in the array
   for (i = 0; i < 7; i++) {
      // if the room name exists in the array, return the index
      if (strcmp(room_array[i].name, search) == 0) {
         return i;
      };
   };  
   return -1;
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
*  HELPFUL REFERENCES FOR MAKING THE PROGRAM
*  1) For creating an array of flags to produce unique random numbers:
*  http://stackoverflow.com/questions/1608181/unique-random-numbers-in-
*  an-integer-array-in-the-c-programming-language
*
*  2) For accessing directories:
*  http://www.gnu.org/software/libc/manual/html_node/Simple-Directory-
*  lister.html
*
*  3) Using fgets with sscanf:
*  http://stackoverflow.com/questions/11168519/fscanf-or-fgets-reading-
*  a-file-line-after-line
*  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
