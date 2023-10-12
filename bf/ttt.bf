++++++++++++++++++++++++++++++++++++++++++++++++  Set @0 to 0x30
>>>++++++++++    Set @3 to \n
<+++    Set @2 to 3

Print 3 lines of values:
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
