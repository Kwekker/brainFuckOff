>++++++[-<++++++++>]<   Set @0 to 0x30 (6 * 8)
>>>++++++++++           Set @3 to \n (0x10)
<+++                    Set @2 to 3

Print 3 lines of values:
[
    <+++ Set @1 to 3
    Print the increasing value of @0 3 times:
    [  
        
        <.+ Move to @0 and print it and increase it
        >-  Move to @1 and decrease it
    ]
    >>.  Print @3 (\n)
    <-  Decrease @2
] 
