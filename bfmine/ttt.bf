[   
    This is supposed to become a 2 player tic tac toe game.
    It's the third time I've tried this since learning about Brainfuck from a guy on youtube named mitxela (https://www.youtube.com/watch?v=qK0vmuQib8Y).
    I hope I can actually finish it this time, using my cool new interpreter.
    I have a pretty clear idea of how this is going to work so it shouldn't be too difficult.
    Made by Jochem Leijenhorst.
    https://github.com/kwekker/brainFuckOff


    Memory:
    @@ 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 >>> BOARD >>>>
    == 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 >>>       >>>>

]

>++++++[-<++++++++>]<   Set @0 to 0x30 (6 * 8)
>>>++++++++++           Set @3 to \n (0x10)
<+++                    Set @2 to 3


Print the 012345678 square
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
#
[   Input checker loop enters at @3
    -           @3 = 0
    <<,         Input at @1
    <[->->+<<]  Suptract 0x30 from input & set @2 to 0x30
    >>[-<<+>>]  @0 = 0x30 from @2

    <[->+>>>+<<<<]  Copy @1(input) to @2 and @5
    -               Set @1 to 0xff (Surprise tool that will help us later)

    Check if it's more than 8:
    >>>++++++++     Set @4 to 8

    Memory now:
     @  00 01 02 03 04 05 06 07
    ==  30 00 ii 00 08 ii 00 00
                    ^^


    #
    [>-[<-<]>]      Check if @5(input) is more than @4(8)
    

    > [ If input is too large:
        ,       Set @5 to trailing \n (FIX MULTIPLE INPUTS!!! I want the input CLEAN!!)
        [-]    @5 = 0
        <<+     @3 = 1 to keep looping
        <[-]    @2 = 0
    ]
    If input was too large we're at @3
    Else we're at @7
    +[-<+]  Use a shifter to get to @1 (We set it to 0xff remember)
            @1 is now 0 btw
    >>      Move to @3
]
Input was good wooo

#
