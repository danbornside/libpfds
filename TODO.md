## The absoltely bleeding edge of what I am currently actually "doing"


* flesh out a mapping typeclass, arraymap instance, treemap instance.

## stuff i'm not doing this instant but i know i should probably have based on immediate needs

* properly clean up `test_pfds.c`
    * `test_num` for numbery things
    * `test_string` for stringey things
    * `test_cateable` for monoidy things
    * `test_fingertree` for fingery details
    * some kinda "this type *should* have this interface, verify that it has that" instead of the current typedef test.
* decruft the .gens field in prop framework shit.  actually we don't need that record at all probably.
* property test that verifies that debugfputs returns the actual number of chars printed.
* test genSize
* verify good coverage of codebase on just properties, just gc, just benchmarks.
* `test_num` module probably.
* `test_catenable` module (ezpz: lIdent, rIdent, lAssoc, rAssoc)
* `test_compareable` module (refl, trans,
* ccheck needs a histogram feature, run a property test until a specified number of bins have been filled to a specified size.

## For this to be in a releasable state, version 0.9 or whatever, i want to have the following things:

* fingertree sequence/mapping
* enforce a strict naming convention:
    * `namespace ${namespace|lower}`
    * `NAMESPACE_MACRONAME ${namespace|upper}_${macro|upper}`
    * `namespace_interFace ${namespace|lower}_${interface|camelCase}`
    * `namespace_interFace_memberFn ${namespace|lower}_${interface|camelCase}_${memberFn|camelCase}`
    * `namespace_TypeName ${namespace|lower}_${typename|lPascalCase}`
    * `namespace_TypeName_memberFn ${namespace|lower}_${typename|lPascalCase}_${memberFn|camelCase}`

* some actual error handling, including *some* sane way to deal with leaked references
* lazy(ier) strings/bytestrings
* boxed versions of major c types
* boxed versions of a few library c types from `<time.h>`
* doxygen docs.


* so uh... goal for today is get the files reorganised
* put the off-brand instances in less visible namespaces:

* `pfds.h` -> `pfds.h` , pfds/pfds-{object,pfds-intl,function}.h
* `string.c`
* `num.c`
* `treelist.c` -> `fingertree.c,treelist.c,pfds/fingertree.h,pfds/treelist.h`
* `arraylist.c` -> `arraylist.c,pfds/pfds-array.h`
* `linkedlist.c` -> `arraylist.c,pfds/pfds-array.h`


----

minor code-tweak todos:

* use a macro for object-header
* probably move struct definitions into c files.
* precise GC tests for every function.
* GPLv3 header on all files (except splitmix, check prior art)

----

* "example code" tests should achieve full coverage
* benchmarks should test use cases of common patterns:
    * stack / fifo queue / prioqueue / builder / random access


-----

ccheck todos: 

 * rename most CCHECK to ccheck
 * size-related stuff.
    * change the size given by test/forAll to be simply the iteration number
    * higher order generators that interact with size according to some rules

       * a schedule that can do some number of tests at a given size or size range.
       * a modifier that can take an environment or configuration multiplier
       * a generator that generates integers related to the given size.
       * a generator that generates an array and some indices that are in bounds for a given size.

some discussion about where i want to take ccheck:

I want to support structs and unions.  to do either i need a way to have child
generators generate their values in place in a predetermined address instead of
always allocating a new heap area and returning apointer to it.  It almost
makes sense to use libffi for this, since we use it anyway for calling; only
libffi doesn't deal with unions, only structs.  so i'll probably need to define
a proper SOP that works sort of the same but with unions.

For actual generation, cleanup will need some way to know which branch of a
union was taken.  That probably means i need to have the dispose method take
the same seed as from generation, so it can generate the same branch.

also, cleanup will be kinda messy; if a user stack allocates a value , then
returns before cleanup, then the value is no longer availalbe to be cleaned up;
so either we can't allow user code to do that kind of allocation, or else we
require the caller do the cleanup before returning, although we can automate a
bit on fatal failing tests since we can do the cleanup before longjmping out.

so the resulting TODOs are, in like reverse chronological order:

 * a decent amount of unit testing for ccheck itself
 * nice builder for union generation
 * a nice builder for struct generation
 * a nice builder for array generation
 * some internal machinery for generic SOP building
 * some internal machinery for "remembering" seeds of samples

and also there's some prior art in eg hypothesis for a gee-wiz regex
geneerator; ie one that generates strings that match a given regex.
