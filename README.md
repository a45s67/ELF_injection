# ELF_research/injection

## Intorduction:

reference video: [IOT malware - NTUSTISC](https://www.youtube.com/watch?v=eUIsggR9Cuo&feature=youtu.be)

上完課後的動手做以及測試記錄

主要是藉由更改ELF，達到控制程式流程，執行注入shellcode，

照著實作並做了一些修改， 增加 PIE bypass 

## Usage:

Hijack entry point version:

`./inject "path_to_victim_binary" "path_to_shellcode" "output_path"`

Hijack GOT table version:

`./got_inject "path_to_victim_binary" "path_to_shellcode" "output_path"`

Hijack plt jump to reslove version:

`Haven't done `

## 常用工具

GDB ,

readelf -a

objdump -d -M intel

pstack

strace

## Some Basic Concepts & Useful Pages:

[Linux Programmar's manual - ELF](http://man7.org/linux/man-pages/man5/elf.5.html)，靠這個網頁，基本能夠理解ELF的架構以及header,section,symbol的關係

[Relocation](https://refspecs.linuxbase.org/elf/gabi4+/ch4.reloc.html), 看到這個我才突然開竅大概知道這些macro是要幹嘛...

### 觀念整理

首先要塞shellcode，就必須塞在可執行段 - LOAD(Program Section)的屁股後面，程式執行時"只會"將LOAD段mapping進memory(我的推測，  
因為其他PS基本跟LOAD的範圍重疊，重複load這些重疊的東西感覺很怪)，總之有兩個LOAD，看flag可以知道xrw的情況。

塞完之後記得把受到影響的header的offset往後推，不然LOAD的時候對不上會壞掉。  
為什麼virtual address(vaddr)不用往後推呢? 我推測因為shellcode是塞在 LOAD(x-r)的屁股，所以map進memory時，shellcode前面的data不受到影響，而後面的LOAD(-wr)則跟前面
前面的LOAD mapping的位置屌差了0x200000的長度，所以後面的data也不會受到影響，所以vaddr，不用動。

有哪些offset要推: Program Header, Section Header(可推可不推，就只是會找不到symbol string和gdb跑不動這樣)。  
推完後 ehdr->e_entry改一下  
然後shellcode後面加個跳回原本main的jmp就ok (課程中是塞絕對位置，感覺這邊可以用相對位置無視PIE，還沒試) 
( 2020/5/10測試，相對位置可成功)


### 塞GOT的測試心得

為了繞過PIE，hijack GOT感覺就是一定要的(?)，但課程中並沒繞過PIE的方法。於是親手測試一發。  
在沒開PIE時，當call xxx@plt時由於GOT表會指到RIP的下一行，然後先跳到gt_...甚麼的把address給reslove進GOT表中，所以思路是我們可以hiject這個jmp的address，改成我們shellcode
的位置，或者是hiject GOT表一開始的內容來控制flow。

有開PIE的問題就是，GOT表在進main時就reslove好了，一開始我是想塞相對位置進GOT然後mapping時reslove好，結果我塞的在進main之前就直接被reslove變成新的(直接puts@plt->IOput)， 
所以我突發奇想，把對應的Sym的struct給搞成0，會不會認不出這是啥symbol就不reslove了，還真的成功。

再更細的測試之後發現，似乎是bind == GLOBAL時的機制導致，所以只要把flag中bind的部分改成LOCAL，elf載入時會給那些相對位置的symbol一個正確的addr並做mapping，  
然後flag被改過，這個sym就不會被dynamic reslove蓋過去(我猜的，詳細順序未知)，就達到了事前hiject GOT的目標。  
(既然不會被dynamic reslove蓋過，感覺可以再試試hiject jmp
