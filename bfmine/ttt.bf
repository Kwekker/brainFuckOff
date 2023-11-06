[   
    This is supposed to become a 2 player tic tac toe game.
    It's the third time I've tried this since learning about Brainfuck from a guy on youtube named mitxela (https://www.youtube.com/watch?v=qK0vmuQib8Y).
    I hope I can actually finish it this time, using my cool new interpreter.
    I have a pretty clear idea of how this is going to work so it shouldn't be too difficult.
    Made by Jochem Leijenhorst.
    https://github.com/kwekker/brainFuckOff


    Memory:
    @@ 00 01 02 03 04 05 06 07 >> BOARD >>
    == 00 00 00 00 00 00 00 00

]

>++++++[-<++++++++>]    Set @0 to 0x30 (6 * 8)
>>++++++++++            Set @3 to \n (0x0a)
<+++                    Set @2 to 3


Print the 123456789 square
[
    <+++ Set @1 to 3
    Print the increasing value of @0 3 times:
    [  
        <.+ Move to @0 and print it and increase it
        >-  Move to @1 and decrease it
    ]
    >>. Print @3 (\n)
    <-  Decrease @2
] 
<<---------     Set @0 to 0x30

>>>[-]+         Set @3 to 1



Input checker loop enters at @3
[   
    -   @3 = 0
    <<, Input at @1
    
    <[->->+<<]      Suptract 0x30 from input & set @2 to 0x30
    >>[-<<+>>]      @0 = 0x30 from @2

    <[->+>>>+<<<<]  Copy @1(input) to @2 and @5
    -               Set @1 to 0xff (Surprise tool that will help us later)

    Check if it's more than 8:
    >>>+++++++++    Set @4 to 9
    Because the comparison requires both inputs to be more than zero             

    Memory now:
     @  00 01 02 03 04 05 06 07
    ==  30 00 ii 00 08 ii 00 00
                    ^^

    [>-[<-<]>]      Check if @5(input) is more than @4(8)


    > [ If input is too large (or too small for that matter):
        >+++++[<+++++++++>-]<++  Add 46 to @5 to check if it's \n
        [,----------]   If @5 != 10: loop until the input is  

        [-]     @5 = 0
        <<+     @3 = 1 to keep looping
        <[-]    @2 = 0
    ]
    If input was too large we're at @3
    Else we're at @7

    +[-<+]  Use a shifter to get to @1 (Which we set to 0xff remember)
            @1 is now 0 btw

    >>      Move to @3
]
Input was good!!

>[-]    @4 = 0
<<<-    Put a shiftlock at @1 for later

Memory now:                    |Board block {M}:
     @  00 01 02 03 04 05 06 07|00 01 02:03 04 05 06 07 08
    ==  30 00 ii 00 00 00 00 00|tt ii ff:SS dd SS dd SS dd etc
                    ^^         |        :Array
    SS are the board states
    dd are delimiters which are used for indexing
    tt is the turn (1 or 0  starts at 0)
    and operations within the array that require memory
                    
>[->>>>>>>+<<<<<<<] Move input to M1
>>>>>>>>->>-        Put a shiftlock at M2 and M4

==== Finding the index ====

<<<     Go to M1
[   -   Loop {input} times
    >>>     Go to first delimiter
    +[->>+] Shift over to first shift lock & set it to 0
    >>-     Move it to the next array delimiter

    <<<<        Go back 2 delimiters
    +[-<<+]-<   Shift back to M1
]

#

==== Changing the index ====

Board block {M}:
00 01 02:03 04 05 06 07 08
tt ii ff:SS dd SS dd SS dd etc
        :Array

This can probably be done better because M0 is either 1 or 0 but oh well
<[-<+<+>>]  Copy M0 to @7 and @6
<[->+<]>    Move @6 to M0

+1 to start the loop
[   - Loop M0 times

    >>>>        Go to first delimiter
    +[->>+]-    Shift to the chosen index

    <[TODO: Handle bruh moment]

    +1 to the val at the index
    <+[-<<+]-<< Shift to M0
]

>>>>        Go to first delimiter
+[->>+]     Shift to and reset the chosen index
<<+[-<<+]-  Shift back to M2