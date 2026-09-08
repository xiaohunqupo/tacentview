Tacent View (this tool) and Tacent are licenced by me (Tristan Grimmer) under the ISC licence. This licence is similar to MIT but with less unnecessary verbiage.

3rd-party SDKs libraries contain their own licences and conditions. For licences used by Tacent see Licence*.txt in the Tacent/Licences directory. The licences for TacentView are a superset of the Tacent licences and are found in the Assets directory.

Thank you to github users Frousties and Oddwarg for providing suggestions/fixes.

LibHEIF and LibDE265 are LGPLv3-licensed libraries included with the Tacent library, a dependency of TacentView. LibHEIF and LibDE265 are protected under LGPLv3. Their use is optional. Obligations when using LGPLv3 libraries in an ISC repository have been met -- in particular supplying source code/linking instructions, this notice, and supplying license text (including the GPLv3). Details, including the exact Commit IDs of both libraries, may be found at [libheif license readme](https://github.com/bluescan/tacent/blob/master/Modules Image/Contrib/LibHEIF/Licence_Readme.txt). To optionally disable support for AVIF and HEIC files and not link TacentView with these two LGPLv3 libraries, comment the following out in CMakeLists.txt before configuring and building.

```
# HEIC/AVIF loading support in Tacent (via LibHEIF).
option(TACENT_ENABLE_HEIF "Build Tacent With LibHEIF Support (HEIC/AVIF)" On)
```

We do not use the GPLv3-licenced work. However we do include a copy of the GPLv3 
licence since it is referred to by LGPLv3. Look in the Assets folder for all
licence copy.
