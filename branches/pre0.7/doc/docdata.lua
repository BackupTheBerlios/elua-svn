-- eLua Documentation structure is defined here

--[[
The menu is defined by an array of { name, link, submenu, title } arrays

'name' can be another array. Names must be the same for all languages, in the same order 
as defined in the "languages" from 'buildall.lua'. If 'name' is a string instead
of a table, it will be used in all languages.

'link' is the relative link corresponding to the menu entry.

'submenu' is optional. If specified, it is an array of arrays, just like the main
menu.

'title' is the title of the HTML page associated with the menu entry (specified as 
an array in different languages or as a string, just like 'name'). If not given, 
it's the same as 'name'.

If 'link' is nil, the entry defines a menu separator.
If 'name' is nil, the entry defines a "hidden menu item": they are part of the template to HTML
generation process, but they don't appear in the menu. These entries NEED A LINK
AND TITLE FIELDS! See the definition of 'forum.html' below for an example of such item.
--]]
local menu =
{
  -- "eLua project" (menu separator)
  { { "eLua project", "Projeto eLua" } },

  -- "Overview"
  { { "Overview", "Apresentação" }, "overview.html", 
    {
      { { "About", "O que é eLua ?" }, "overview.html#whatis" },
      { { "Features", "Características" }, "overview.html#features" },
      { { "Audience", "Público Alvo" }, "overview.html#audience" },
      { { "Authors", "Autores" }, "overview.html#authors" },
      { { "Contacts","Contatos" }, "overview.html#contacts" },
      { { "License", "Licença" },"overview.html#license" }
    }
  },

  -- "Community"
  { { "Community", "Comunidade" }, "comunity.html", 
    {
      { { "Lists", "Listas de Discussão" }, "comunity.html#lists" },
      { "Forums", "comunity.html#forums" },
--      { "User's Wiki", "comunity.html#userswiki" },      
      { { "Credits", "Créditos" }, "comunity.html#credits" },
      { nil, "forum.html", nil, "Forum" } -- "hidden" entry (doesn't appear in the menu)
    }
  },

  -- News
  { { "News", "Notícias" }, "news.html" },

  -- "Status"
  { "Status", "status.html", 
    {
      { { "Supported platforms", "Plataformas Suportadas" }, "status.html#platforms" },
      { { "Roadmap", "Planejamento Futuro" }, "status.html#roadmap" }
    }
  },

 -- "Using eLua" -- Menu Separator
  { { "Using eLua", "Usando eLua"}, nil },      

  { "Downloads", "downloads.html",    -- ### How come it opens downloads.html without specifying here ? :) Nice but unclear.
    {
      { { "Binaries", "Binários" }, "downloads.html#binaries" },
      { { "Source code", "Código Fonte" }, "downloads.html#source", 
        {
          { { "Official releases", "Versões Oficiais" }, "downloads.html#official" },
          { "Bleeding edge (SVN)", "downloads.html#svnpublic" },
          { { "Developers", "Desenvolvedores" }, "downloads.html#svndev" },
        }
      },
      { { "Old versions", "Versões Anteriores" }, "dl_old.html", 
        {
          { "0.5", "dl_old.html#v050" },
          { "0.4.1", "dl_old.html#v041" },
          { "0.4", "dl_old.html#v04" },
          { "0.3", "dl_old.html#v03" },
          { "0.2", "dl_old.html#v02" },
          { "0.1", "dl_old.html#v01" },
          { { "Version history", "Histórico das Versões" }, "versionhistory.html" },
        }
      },
    },
  },
  { { "Generic info", "Ajuda" }, "using.html", 
    {
      { { "Using eLua", "Usando eLua" }, "using.html", 
        {
          { { "Over UART", "Console via UART" }, "using.html#uart" },
          { { "Over TCP/IP", "Console via TCP/IP" }, "using.html#tcpip" },
          { { "On PC", "Num PC" }, "using.html#pc" },
          { { "The shell", "O Shell" }, "using.html#shell" },
          { "Cross-compiling", "using.html#cross" },
          { "LuaRPC", "using.html#rpc" },
          { "eLua and EGC", "elua_egc.html" },
        },
      },
      { { "Building eLua", "Build de eLua" }, "building.html",
        {
          { "Building eLua in Linux", "building_unix.html" },
          { "Building eLua in Windows", "building_win.html" },
        },  
      },
      { { "Code examples", "Exemplos de Código" }, "examples.html" },
      { { "FAQ", "Perguntas Frequentes (FAQ)" }, "faq.html" },     
      { "eLua file systems", "filesystems.html",
        {
          { { "ROM file system", "O ROM File System" }, "arch_romfs.html" },
          { "FAT file system", "fatfs.html" }
        }
      },
      { { "Tutorials", "Tutoriais" }, "tutorials.html",
        {
          { { "Booting on a PC", "Bootando num PC" }, "tut_bootpc.html" },
          { { "Booting from a PenDrive", "Bootando de um Pen-Drive" }, "tut_bootstick.html" },
          { { "Using OpenOCD", "Usando OpenOCD" }, "tut_openocd.html" },
          { { "eLua toolchains", "Toolchains para eLua" }, "toolchains.html" },
          { { "Building toolchains", "Build de Toolchains" }, "tchainbuild.html",
            {
              { "ARM7 and ARM9", "tc_arm.html" },
              { "ARM Cortex-M3", "tc_cortex.html" },
              { "i386", "tc_386.html" }
            },
          },
        },
      },
    },
  }, 
  { { "Platform info", "Plataformas" }, "installing.html", 
    {
      { "AT91SAM7x", "installing_at91sam7x.html" },
      { "AVR32", "installing_avr32.html" },
      { "i386", "installing_i386.html" },
      { "LM3S", "installing_lm3s.html" },
      { "LPC2888", "installing_lpc2888.html" },
      { "STR7", "installing_str7.html" },
      { "STR9", "installing_str9.html" },
      { "STM32", "installing_stm32.html" },
    },
  },        
          
  
  -- "eLua internals" (menu separator)
  { { "eLua internals", "Arquitetura de eLua" } },

  -- "Overview"
  { { "Overview", "Visão Geral" }, "arch_overview.html", 
    {
      { { "Structure", "Estrutura" }, "arch_overview.html#structure" },
      { { "Common code", "Código Básico" }, "arch_overview.html#common" },
      { { "Interface architecture", "Interfaceamento" }, "arch_overview.html#platform" },
      { { "Platforms/ports", "Portabilização" }, "arch_overview.html#platforms" },
      { { "Booting eLua", "O Boot de eLua" }, "arch_overview.html#boot" }
    }
  },

  -- Platform interface (automatically generated)
  { { "Platform interface", "Interfaceamento" }, "arch_platform.html", "arch_platform" },

  -- Other WRITE THE arch_platform_other.html file
  { { "More information", "Informações Adicionais" }, "#", 
    {
      { { "Adding a new port", "Portando eLua" }, "arch_newport.html" },
      { { "Modules and LTR", "Modulos e LTR" }, "arch_ltr.html" },
      { { "Consoles and terminals", "Consoles e Terminais" }, "arch_con_term.html" },
      { { "TCP/IP in eLua", "TCP/IP em eLua" }, "arch_tcpip.html" },
      { { "eLua coding style", "Regras de Codificação" }, "arch_coding.html" }
    }
  },

  -- "Reference manual" (menu separator)
  { { "Reference Manual", "Manual de Referência" } },

  -- "Generic modules" (automatically generated)
  { { "Generic modules", "Módulos Genéricos" }, "refman_gen.html", "refman_gen" },

  -- "Platform modules"
  { { "Platform modules", "Módulos Específicos" }, "refman_dep.html", 
    {
      { "lm3s", "modules_lm3s.html", "refman_ps_lm3s" }
    }
  }
}


-- Translations for different strings needed by the generator
-- The order of languages is the same as the one defines in the languages array 
-- defined at the beginning of buildall.lua
local translations = 
{
  { "Overview", "Apresentação" },
  { "Data structures", "Estrutura" },
  { "Functions", "Funções" },
  { "eLua - Embedded Lua", "eLua - Embedded Lua" },
  { "Search", "Pesquisar" },
  { "Language", "Idioma" }
}

-- The "fixed part" are the files/directory that must be present in our final
-- HTML documentation distribution. Directories end with a "/", files don't.
local fixed = 
{
  "style1.css",
  "menu.css",
  "images/",
  "ddlevelsfiles/"
}

-- Return our whole data
return menu, translations, fixed

