# Parser
> **Pipeline**
![](workstream.jpg)

> **Grammer Defination**
>>*context-free grammer* \
G[s] := <V<sub>N</sub>, V<sub>T</sub>, P, S>

>>V<sub>N</sub> = {} \
V<sub>T</sub> = {} \
S = {}


    Cpp Productions::

    <declaration> -> [<declaration_descriptor>]<R1>
    <R1> -> <declaration_notion><R2>|;
    <R2> -> <R5><R4>; | [<declaration_list>]<composition_sentence>
    <R4> -> <R3><R4> | epsilon 
    <R5> -> ,<declaration_init>
    <R5> -> =<content_init> | epsilon 

    <declaration_init> -> <declaration_notion><R5>

    <declaration_descriptor> -> <store_type>[<declaration_descriptor>]
                                | <type>[<declaratioin_descriptor>]
                                | <type_constrant>[<declaratioin_descriptor>]
                                | <func>[<declaratioin_descriptor>]
                                | <algin>[<declaratioin_descriptor>]


    <declaration_notion> -> [<pointer>]<direct_declaration>

    <type> -> int | long | char | double | ...

    <direct_declaration> -> <identifier><T4>
                            | ( <declaration_notion> )<T4>

    <T4> -> <T1> | <T5> | epsilon
    <T1> -> <array_declaration> T1 | epsilon
    <T5> -> ( <T5> )
    <T6> -> <params_type_list> | <identifier_list> | epsilon

    <array_declaration> -> [ [type_constrant_list][<copy_expression>] ]

    <params_type_list> -> <param_list><T9>

    <T9> -> , ... | epsilon

    <params_list> -> <params_declaration><T10>
    <T10> -> ,<params_list> | epsilon

    <params_declaration> -> <declaration_descriptor><declaration_notion>


> **We use json to load the grammer of the language** 
>> **V<sub>N</sub>** :: *non_terminals = {}* \
>> **V<sub>T</sub>** :: *terminals = {}* \
>> **S** :: *start_symbols = {}* \
>> **productions** ={
>>          <p>&emsp; &emsp;*{ E -> E + T | E - T | T }*, \
>> &emsp; &emsp;            *{ T -> T * F | T / F | F }*, \
>> &emsp; &emsp;            *{ F -> ( E ) | num }*</p>
    }  

> ***grammer.json***
```json
{
    "Type": "Grammer",
    "non_terminals": "E T F",
    "terminals": "+ - * / ( ) num",
    "start_symbol": "E",
    "productions": [
        {
            "left": "E",
            "candidate": [
                "E+T",
                "E-T",
                "T"
            ]
        },
        {
            "left": "T",
            "candidate":[
                "T*F",
                "T/F",
                "F"
            ]
        },
        {
            "left": "F",
            "candidate": [
                "(E)",
                "num"
            ]
        }
    ]
}
```

> ***Eliminate the left recursion***
> - direct left recursion
> - indirect left recursion
-
    **left recursion** ::
    ![fomula](1.jpg)
    ![fomula](2.jpg)
- 
  **if there exists many productions that include left recursion** \
  we should reorder if and eliminate all the productions contains left recursion 
- **code as follow**
    ```

    ```
> ***Extract the left common factor***
![fomula](3.jpg)
- **code as follow**
    ```

    ```


> ***First Set***
<!-- $$First(\alpha) \subset Follow(A)\space , which\space A\rightarrow\alpha \space ,\space \alpha \space not\space \epsilon$$
$$If\space \alpha \space is \space \epsilon ,\space \epsilon \in \space First(A)$$
$$If\space \alpha \in V_T, \space First(\alpha) =\{ \alpha \}$$ -->
![formula](4.jpg)
- code as follow
```C++

```

> ***Follow Set***
<!-- $$if\space exsits\space A\rightarrow \alpha B\beta\space $$
$$if\space \beta {\Rightarrow}^* \epsilon$$
$$ Follow(A) \subset Follow(B)$$
$$ else\space if\space \beta {\nRightarrow} \epsilon $$
$$ First(\beta) \subset Follow(B) $$  -->
![formula](5.jpg)
- code as follow
```C++

```

> ***LL(1) Table***
>>
|        LL(1)         |       terminals    |
|       :----:         |        :----:      |
|      non_terminals   |      productions   |