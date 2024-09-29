# DeDuper
De-cuplicate your files in  C++/MFC.  Bitwise comparison, no smart stuff here.  It relies on file system buffering, only reading one byte at a time.  So huge files are no problem, but it is single-threaded.

C++/MFC simple local app.  Don't spend money to get this kind of thing, and if you have a later port like c# then very jolly of you.

1 and 2 select individual files for revycling.  No side effects.

Letters: You select the folder where files will be DELETED (actually just moved to the Recycle bin). To keep folder A files, would click B for the file in folder B, and then click B for the file in folder C. It will always recycle dupe files in B and C.

Note: it will not care about with is the "other" folder. That is, it won't associate folder B or C with folder A. Once you select folder C, it will recycle files in that folder regardless. The mentality is "I don't want that folder". 

"A" for "Always this folder". Similar with V and W. V is for "NeVer delete files in this folder". W is the same for the other file.  A for Always, B fo Blways.  V for neVer, w for neWer.

I'd recommend setting up a few test folders to make sure it works the way you expect. And then on very small samples at a time.

I made it for me,, a keyboard warrior, not really to share. So if you test it and find things that would make it more intuitive, open an issue.
