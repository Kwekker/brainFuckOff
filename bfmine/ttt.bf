[   
    This is supposed to become a 2 player tic tac toe game.
    It's the third time I've tried this since learning about Brainfuck from a guy on youtube named mitxela (https://www.youtube.com/watch?v=qK0vmuQib8Y).
    I hope I can actually finish it this time, using my cool new interpreter.
    I have a pretty clear idea of how this is going to work so it shouldn't be too difficult.
    Made by Jochem Leijenhorst.
    https://github.com/kwekker/brainFuckOff
]
>++++++[-<++++++++>]<   Set @0 to 0x30 (6 * 8)
>>>++++++++++           Set @3 to \n (0x10)
<+++                    Set @2 to 3

Print the 02345678 square
[
    <+++ Set @1 to 3
    Print the increasing value of @0 3 times:
    [  
        #
        <.+ Move to @0 and print it and increase it
        >-  Move to @1 and decrease it
    ]
    >>.  Print @3 (\n)
    <-  Decrease @2
] 
>[-]<       Set @3 back to 0
<,          Input at @1
<---------  Set @0 back to 0x30
[->-<]      Suptract 0x30 from input
>[->+>+<<]  Copy @1(input) to @2 and @3

Check if it's more than 8:
+++++++++   Set @1 to 9
[

]
