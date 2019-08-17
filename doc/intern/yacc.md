<meta charset="utf-8">

# 编译器教程系列：[1] YACC ParserGen，从0到0.1

## 前言
我天泽一向偏向于手写解析器代码，觉得把一切掌握在自己手里才是最好的。
不过最近突发奇想，打算开发一门DSL，名为PiLang。该语言致力于减少Shift键的使用。
一个简单的a + b程序写起来就像下面这样：`

    function main takes returns
    begin
      a = input [];
      b = input [];
      print [ a + b ];
    end function

因为语言本身语法非常简单，而我又没有那么多时间去进行实际的编写工作
<del>胡说八道，操作系统上机课时间全都被你拿来写这个了</del>，
所以我决定利用现有的Parser生成器来代劳。

因为PiLang是一种需要运行在低端计算机上的编程语言，
所以我不想使用Python这样的“高端”语言来完成工作，甚至连Java都不想；
不然我早就用EBNFParser了。我决定利用古老的yacc来生成我所需要的parser。

在搜索资料的时候，我在国内外论坛上找到了若干关于yacc/bison
（或多或少也关于lex/flex）的介绍。但是这部分资料大都浅尝辄止，
完全满足不了我的需要。
最终我在[lex+yacc page](http://dinosaur.compilertools.net/)
上找到了我所需要的部分信息。我为这部分信息建立了镜像，
读者可以在[这里](http://118.24.124.97/contents/lex+yacc.zip)
下载到这部分内容。

在这之后，我又将来自论坛各处的信息进行了整合，最终做出了一个基本可用的Parser。
我希望把我的学习过程分享出来，供后来者进行参考。

## YACC
__YACC__是__Y__et __A__nother __C__ompiler __C__ompiler的缩写。
它接收用户输入的语法说明文件，并且根据语法说明生成一份Parser。
据文献记载YACC能和多种语言共同协作。本文中我们用YACC与C语言协作，
让YACC生成C语言的Parser。总的流程大概是这样：

    +--------+   yacc    +---------+  cc   +--------+
    | some.y |  ------>  | y.tab.c | ----> | parser |
    +--------+           +---------+       +--------+

## YACC语法说明文件
一个YACC语法说明文件由以下三个部分组成：

    声明
    %%
    规则
    %%
    附加程序

### 规则部分
规则部分最为重要：
程序员在这部分用一系列“产生式”<sup>[1]</sup>定义输入文件的__文法__，就像这样：

    expr: expr TK_SYM_ADD term;
    expr: expr TK_SYM_SUB term;
    expr: term;
    term: term TK_SYM_MUL TK_NUMBER;
    term: term TK_SYM_DIV TK_NUMBER;
    term: TK_NUMBER;

> <sup>[1]</sup> “产生式”是龙书中的说法，
产生式的含义是产生式“左部”的非终止符号可以由产生式右部的符号组成。<br/>
> <sup>[2]</sup> 终止符号(terminal symbol，又译终端符号，终结符号等)，
非终止符号(non-terminal symbol，又译非终端符号，非终结符号等) <br/>

这里我们约定全大写的标识符是终止符号，全小写的标识符是非终止符号：
这也是YACC所推荐的书写方式。终止符号代表着由词法分析而来的Token，
而非终止符号代表着语法分析过程的中间结果。

规则部分的第一个产生式左部的非终止符号具有特殊的含义：它是一个“开始”符号。

反复写下产生式左部并不是一个明智的决定。更好的方法是使用`|`符号：

    expr: expr TK_SYM_ADD term
          | expr TK_SYM_SUB term
          | term ;

我们还可以为产生式附上一些“动作”，就像：

    expr: expr TK_SYM_ADD term { printf("Add!\n"); }
          | expr TK_SYM_SUB term { printf("Substract!\n"); }
          | term ;

当YACC识别出产生式右部时，对应的语义动作就会执行。
这里我们先做一个非常简单的例子，我们把我们所识别出的信息打印出来。
下附本节完整代码：

    expr: expr TK_SYM_ADD term { printf("Add!\n"); }
          | expr TK_SYM_SUB term { printf("Substract!\n"); }
          | term ;

    term: term TK_SYM_MUL TK_NUMBER { printf("Multiply!\n"); }
          | term TK_SYM_DIV TK_NUMBER { printf("Divide!\n"); }
          | TK_NUMBER { printf("Number!\n"); } ;

### 声明部分
事实上，在YACC的语法说明文件中，除了规则部分是必须的之外，
其他部分都是“可有可无”的。你完全可以写出一份只有规则的语法说明文件
（第二组`%%`可以省略，如果你不写附加程序）：

    %%
    规则

现在你可以尝试一下，将上一小节中给出的规则写到YACC的语法描述里，保存该文件，
然后用YACC处理它：

    $ yacc some.y

不出意外，你会收到一大堆编译错误：

    pi@raspberrypi:~/program $ yacc some.y
    some.y:6.23-31: error: symbol TK_NUMBER is used, but is not defined as a token and has no rules
      term: term TK_SYM_MUL TK_NUMBER { printf("Multiply!\n"); }
                            ^^^^^^^^^
    some.y:2.12-21: error: symbol TK_SYM_ADD is used, but is not defined as a token and has no rules
      expr: expr TK_SYM_ADD term { printf("Add!\n"); }
                 ^^^^^^^^^^
    some.y:7.14-23: error: symbol TK_SYM_DIV is used, but is not defined as a token and has no rules
            | term TK_SYM_DIV TK_NUMBER { printf("Divide!\n"); }
                   ^^^^^^^^^^
    some.y:6.12-21: error: symbol TK_SYM_MUL is used, but is not defined as a token and has no rules
              term: term TK_SYM_MUL TK_NUMBER { printf("Multiply!\n"); }
                         ^^^^^^^^^^
    some.y:3.14-23: error: symbol TK_SYM_SUB is used, but is not defined as a token and has no rules
            | expr TK_SYM_SUB term { printf("Substract!\n"); }
                   ^^^^^^^^^^

为什么会这样呢？如你所见，你已经定义了所有的非终结符号由什么来组成，
但是你却没有定义它们的根基——所有的终结符号，
应该由词法分析器产生的`token`s，你都没有定义。

在这种时候，声明部分就派的上用场啦，这部分就是为声明这种东西而设的！

    %token TK_SYM_ADD TK_SYM_SUB TK_SYM_MUL TK_SYM_DIV TK_NUMBER

`%token`告诉YACC，“以下都是终结符号，是从lexer那边远道而来的token！”，
YACC就会明白自己没必要处理它们，只要知道它们是终结符号就可以了。
现在试着将上述代码添加到`some.y`的声明部分试试看吧。

### 引入外部组件
当你定义完tokens之后，尝试去用yacc生成你的parser:

    $ yacc some.y

YACC应该已经成功处理了你的输入，并且生成了一个Parser。
它默认的名字应该是`y.tab.c`。现在编译它试试？

    $ gcc y.tab.c

哦天啊，刚刚解决完一大堆编译问题，紧接着又来了一大堆编译问题！

    y.tab.c: In function ‘yyparse’:
    y.tab.c:1118:16: warning: implicit declaration of function ‘yylex’
           yychar = yylex ();
                    ^~~~~
    some.y:4:7: warning: implicit declaration of function ‘printf’
    expr: expr TK_SYM_ADD term { printf("Add!\n"); }
           ^~~~~~

    ...

    some.y:10:7: note: include ‘<stdio.h>’ or provide a declaration of ‘printf’
    y.tab.c:1277:7: warning: implicit declaration of function ‘yyerror’; did you mean ‘yyerrok’?
           yyerror (YY_("syntax error"));
           ^~~~~~~
           yyerrok
    /usr/lib/gcc/x86_64-linux-gnu/7/../../../x86_64-linux-gnu/Scrt1.o：在函数‘_start’中：
    (.text+0x20)：对‘main’未定义的引用
    /tmp/ccSE4kgT.o：在函数‘yyparse’中：
    y.tab.c:(.text+0x252)：对‘yylex’未定义的引用
    y.tab.c:(.text+0x529)：对‘yyerror’未定义的引用
    y.tab.c:(.text+0x664)：对‘yyerror’未定义的引用

你应该能理解到YACC只是一个“生成器”，它会接收你给的“语法说明”，
并且根据你的要求生成一份C语言的Parser。事实上，
这份Parser里除了一个光秃秃的yyparse函数什么都没有。
现在来审视一下我们面临的编译问题：

首先是`implicit declaration of function 'printf'`：
你在你的“动作”部分里面用到了`stdio.h`中定义的
标准库函数，但是yacc不会帮你引用`stdio.h`，
YACC只会把你的“动作”代码原样插入到它生成的parser里，就像：

    if (some pattern is matched) { /* your actions */ }

所以你得自己引用你的代码所用到的头文件。声明部分再次起到作用了：

    %{
      #include <stdio.h>
    %}
    ... // other declarations

    %%
    ... // rules

包含在`%{`和`%}`之间的部分会被直接插入到生成的Parser中。
当心，YACC在这里“空白敏感”，不要在`%{`和`%}`前面插入__任何__空格。

yylex和yyerror缺少声明的问题也可以如法炮制，在`%{`和`%}`插入相应的声明即可。

### 附加程序
在上面报错信息的最后是一大批链接错误，这类错误可以归结为“函数只有声明没有定义”。
确实，我们好像并没有去管main函数和yyerror函数。

一般而言，这部分代码应该写在YACC语法说明文件的第三个部分“附加程序”中，就像这样：

    %{
        // 引用
    %}
    ... // 声明
    %%
    ... // 规则
    %%
    void yyerror(const char* text) {
      printf("error: %s!\n", text);
    }

    int main() {
      yyparse();
    }

_如果我们在gcc编译时指定一个链接选项-y，那么gcc就会把我们的程序和一个“默认”的
library连接，这个library中包含了默认的主函数和yyerror。_

### Lexer
到此为止，我们只剩下一个Lexer(`yylex`)没有解决。因为我们要做的词法分析工作非常简单，
所以我们直接手写一份yylex。同样，这部分内容也应该被写在“外部程序”部分当中
（当然，也可以写在其他文件里，然后和生成出的Parser链接在一起）
