SHARC - Linux kernel module
===========================

SHARC ( Simple Highspeed Archiver ) is a very fast file archiver with one goal in mind : speed. 

On an Intel Core i7 it reaches compression speeds of <b>460 MB/s</b> and decompression speeds of <b>770 MB/s</b>, and that is <b>PER core</b> !
Compression ratio is typically at around 50-60 % with the fastest algorithm.

For more information on Sharc please refer to www.centaurean.com/sharc

The Linux kernel module project
-------------------------------

This project can be seen as a Sharc subproject. It does not intend to work on the core program itself.
It has been created in order to adapt Sharc and make it work as a compression provider in the GNU/Linux kernel.

Ultimately we intend to test sharc as the compressor for the zswap mechanism which should has been introduced in GNU/Linux kernel 3.11.

This is the second attempt of doing this work (first was a branch in centaurean/sharc repos). It should take full advantage of the new API.

Plans
-----

* Use kernel coding standards
* Make a device to test proper operation from user space
* Add sysfs entries to test various parameters/read stats    
* Tests once in a while
* Integrate in kernel build tree  
* Test as compressor for zswap  

Done
----
* Remove user space API references
* Integrate in the crypto API
* Build as a standalone module

To do
-----

Still allmost everything :/
