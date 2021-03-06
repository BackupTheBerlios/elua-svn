-- eLua build system

module( ..., package.seeall )

local lfs = require "lfs"
local sf = string.format

-- Taken from Lake
local dir_sep = package.config:sub( 1, 1 )
local is_windows = dir_sep == '\\'

-------------------------------------------------------------------------------
-- Various helpers

-- Return the time of the last modification of the file
local function get_ftime( path )
  local t = lfs.attributes( path, 'modification' )
  return t or -1
end

-- Check if a given target name is phony
local function is_phony( target )
  return target:find( "#phony" ) == 1
end

-- Return a string with $(key) replaced with 'value'
local function expand_key( s, key, value )
  local fmt = sf( "%%$%%(%s%%)", key )
  return ( s:gsub( fmt, value ) )
end

-- Return a target name considering phony targets
local function get_target_name( s )
  if not is_phony( s ) then return s end
end

-- 'Liniarize' a file name by replacing its path separators indicators with '_'
local function linearize_fname( s )
  return ( s:gsub( "[\\/]", "__" ) )
end

-- Helper: transform a table into a string if needed
local function table_to_string( t )
  if not t then return nil end
  if type( t ) == "table" then t = table.concat( t, " " ) end
  return t
end

---------------------------------------
-- Table utils 
-- (from http://lua-users.org/wiki/TableUtils)

function table.val_to_str( v )
  if "string" == type( v ) then
    v = string.gsub( v, "\n", "\\n" )
    if string.match( string.gsub(v,"[^'\"]",""), '^"+$' ) then
      return "'" .. v .. "'"
    end
    return '"' .. string.gsub(v,'"', '\\"' ) .. '"'
  else
    return "table" == type( v ) and table.tostring( v ) or tostring( v )
  end
end

function table.key_to_str ( k )
  if "string" == type( k ) and string.match( k, "^[_%a][_%a%d]*$" ) then
    return k
  else
    return "[" .. table.val_to_str( k ) .. "]"
  end
end

function table.tostring( tbl )
  local result, done = {}, {}
  for k, v in ipairs( tbl ) do
    table.insert( result, table.val_to_str( v ) )
    done[ k ] = true
  end
  for k, v in pairs( tbl ) do
    if not done[ k ] then
      table.insert( result,
        table.key_to_str( k ) .. "=" .. table.val_to_str( v ) )
    end
  end
  return "{" .. table.concat( result, "," ) .. "}"
end


-------------------------------------------------------------------------------
-- Public utilities

utils = { dir_sep = dir_sep }

-- Converts a string with items separated by 'sep' into a table
utils.string_to_table = function( s, sep )
  if type( s ) ~= "string" then return end
  sep = sep or ' '
  if s:sub( -1, -1 ) ~= sep then s = s .. sep end
  s = s:gsub( sf( "^%s*", sep ), "" )
  local t = {}
  local fmt = sf( "(.-)%s+", sep )
  for w in s:gmatch( fmt ) do table.insert( t, w ) end
  return t
end

-- Split a file name into 'path part' and 'extension part'
utils.split_path = function( s )
  local pos
  for i = #s, 1, -1 do
    if s:sub( i, i ) == "." then
      pos = i
      break
    end
  end
  if pos then return s:sub( 1, pos - 1 ), s:sub( pos ) end
  return s
end

-- Replace the extension of a give file name
utils.replace_extension = function( s, newext )
  local p, e = utils.split_path( s )
  if e then s = p .. "." .. newext end
  return s
end

-- Return 'true' if building from Windows, false otherwise
utils.is_windows = function()
  return is_windows
end

-- Prepend each component of a 'pat'-separated string with 'prefix'
utils.prepend_string = function( s, prefix, pat )  
  if not s or #s == 0 then return "" end
  pat = pat or ' '
  local res = ''
  local st = utils.string_to_table( s, pat )
  utils.foreach( st, function( k, v ) res = res .. prefix .. v .. " " end )
  return res
end

-- Like above but consider 'prefix' a path
utils.prepend_path = function( s, prefix, pat )
  return utils.prepend_string( s, prefix .. dir_sep, pat )
end

-- full mkdir: create all the paths needed for a multipath
utils.full_mkdir = function( path )
  local ptables = utils.string_to_table( path, dir_sep )
  local p, res = ''
  for i = 1, #ptables do
    p = ( i ~= 1 and p .. dir_sep or p ) .. ptables[ i ]
    res = lfs.mkdir( p )
  end
  return res
end

-- Concatenate the given paths to form a complete path
utils.concat_path = function( paths )
  return table.concat( paths, dir_sep )
end

-- Return true if the given array contains the given element, false otherwise
utils.array_element_index = function( arr, element )
  for i = 1, #arr do
    if arr[ i ] == element then return i end
  end
end

-- Linearize an array with (possibly) embedded arrays into a simple array
utils._linearize_array = function( arr, res )
  if type( arr ) ~= "table" then return end
  for i = 1, #arr do
    local e = arr[ i ]
    if type( e ) == 'table' then
      utils._linearize_array( e, res )
    else
      table.insert( res, e )
    end
  end 
end

utils.linearize_array = function( arr )
  local res = {}
  utils._linearize_array( arr, res )
  return res
end

-- Return an array with the keys of a table
utils.table_keys = function( t )
  local keys = {}
  utils.foreach( t, function( k, v ) table.insert( keys, k ) end )
  return keys
end

-- Returns true if 'path' is a regular file, false otherwise
utils.is_file = function( path )
  return lfs.attributes( path, "mode" ) == "file"
end

-- Return a list of files in the given directory matching a given mask
utils.get_files = function( path, mask )
  local t = ''
  for f in lfs.dir( path ) do
    local fname = path .. dir_sep .. f
    if lfs.attributes( fname, "mode" ) == "file" and fname:find( mask ) then
      t = t .. ' ' .. fname
    end
  end
  return t
end

-- Check if the given command can be executed properly
utils.check_command = function( cmd )
  local res = os.execute( cmd .. " > .build.temp 2>&1" )
  os.remove( ".build.temp" )
  return res
end

-- Execute the given command for each value in a table
utils.foreach = function ( t, cmd )
  if type( t ) ~= "table" then return end
  for k, v in pairs( t ) do cmd( k, v ) end
end

-------------------------------------------------------------------------------
-- Dummy 'builder': simply checks the date of a file

local _fbuilder = {}

_fbuilder.new = function( target, dep )
  local self = {}
  setmetatable( self, { __index = _fbuilder } )
  self.target = target
  self.dep = dep
  return self
end

_fbuilder.build = function( self )
 -- Doesn't build anything but returns 'true' if the dependency is newer than
 -- the target
 if is_phony( self.target ) then
   return true
 else
   return get_ftime( self.dep ) > get_ftime( self.target )
 end
end

_fbuilder.target_name = function( self )
  return get_target_name( self.dep )
end

-- Object type
_fbuilder.objtype = function()
  return "_fbuilder"
end

-------------------------------------------------------------------------------
-- Target object

local _target = {}

_target.new = function( target, dep, command, builder )
  local self = {}
  setmetatable( self, { __index = _target } )
  self.target = target
  self.command = command
  self:set_dependencies( dep )
  self._force_rebuild = #self.dep == 0
  self.builder = builder
  builder.tlist[ target ] = self
  builder.runlist[ target ] = false
  return self
end

-- Set dependencies
_target.set_dependencies = function( self, dep )
  -- Transform 'dep' into a list of objects that support the 'build' method
  if type( dep ) == 'string' then 
    dep = utils.string_to_table( dep ) 
  elseif type( dep ) ~= 'table' then
    dep = {}
  end
  -- Iterate through 'dep' transforming file names in _fbuilder targets
  for i = 1, #dep do
    if type( dep[ i ] ) == "string" then
      dep[ i ] = _fbuilder.new( self.target, dep[ i ] )
    end
  end
  self.dep = dep
end

-- Set pre-build function
_target.set_pre_build_function = function( self, f )
  self._pre_build_function = f
end

-- Set post-build function
_target.set_post_build_function = function( self, f )
  self._post_build_function = f
end

-- Force rebuild
_target.force_rebuild = function( self, flag )
  self._force_rebuild = flag
end

-- Set additional arguments to send to the builder function if it is a callable
_target.set_target_args = function( self, args )
  self._target_args = args
end

-- Function to execute in clean mode
_target._cleaner = function( target, deps, dummy )
  -- Clean the main target if it is not a phony target
  if not is_phony( target ) then 
    io.write( sf( "[builder] Removing %s ... ", target ) )
    if os.remove( target ) then print "done." else print "failed!" end
  end
  return 0
end

-- Build the given target
_target.build = function( self )
  if self.builder.runlist[ self.target ] then return end
  local docmd = self:target_name() and lfs.attributes( self:target_name(), "mode" ) ~= "file"
  docmd = docmd or self.builder.global_force_rebuild
  local initdocmd = docmd
  local depends, dep = '', self.dep
  -- Iterate through all dependencies, execute each one in turn
  local deprunner = function()
    for i = 1, #dep do
      local res = dep[ i ]:build()
      docmd = docmd or res
      local t = dep[ i ]:target_name()
      if dep[ i ]:objtype() == "_target" and t then
        docmd = docmd or get_ftime( t ) > get_ftime( self.target )
      end
      if t then depends = depends .. t .. " " end
    end
  end
  deprunner()
  -- Execute the preb-build function if needed
  if self._pre_build_function then self._pre_build_function( self, docmd ) end
  -- If the dependencies changed as a result of running the pre-build function
  -- run through them again
  if dep ~= self.dep then
     depends, dep, docmd = '', self.dep, initdocmd
     deprunner()
  end
  -- If at least one dependency is new rebuild the target
  docmd = docmd or self._force_rebuild or self.builder.clean_mode
  local keep_flag = true
  if docmd and self.command then
    local cmd, code = self.command
    if self.builder.clean_mode then cmd = _target._cleaner end
    if type( cmd ) == 'string' then
      cmd = expand_key( cmd, "TARGET", self.target )
      cmd = expand_key( cmd, "DEPENDS", depends )
      cmd = expand_key( cmd, "FIRST", dep[ 1 ]:target_name() )
      print( cmd )
      code = os.execute( cmd )   
    else
      code = cmd( self.target, self.dep, self._target_args )
      if code == 1 then -- this means "mark target as 'not executed'"
        keep_flag = false
        code = 0
      end
    end
    if code ~= 0 then 
      print( "[builder] Error building target" )
      os.exit( 1 ) 
    end
  end
  -- Execute the post-build function if needed
  if self._post_build_function then self._post_build_function( self, docmd ) end
  -- Marked target as "already ran" so it won't run again
  self.builder.runlist[ self.target ] = true
  return docmd and keep_flag
end

-- Return the actual target name (taking into account phony targets)
_target.target_name = function( self )
  return get_target_name( self.target )
end

-- Object type
_target.objtype = function()
  return "_target"
end

-------------------------------------------------------------------------------
-- Builder public interface

builder = { KEEP_DIR = 0, BUILD_DIR = 1, BUILD_DIR_LINEARIZED = 2 }    

---------------------------------------
-- Initialization and option handling

-- Create a new builder object with the output in 'build_dir' and with the 
-- specified compile, dependencies and link command
builder.new = function( build_dir )
  self = {}
  setmetatable( self, { __index = builder } )
  self.build_dir = build_dir or ".build"
  self.exe_extension = utils.is_windows() and "exe" or ""
  self.clean_mode = false
  self.options = {}
  self.args = {}
  self.build_mode = self.KEEP_DIR
  self.targets = {}
  self.targetargs = {}
  self.tlist = {}
  self.runlist = {}
  return self
end

-- Argument validator: boolean value
builder._bool_validator = function( v )
  if v == '0' or v:upper() == 'FALSE' then
    return false
  elseif v == '1' or v:upper() == 'TRUE' then
    return true
  end
end

-- Argument validator: choice value
builder._choice_validator = function( v, allowed )
  for i = 1, #allowed do
    if v:upper() == allowed[ i ]:upper() then return allowed[ i ] end
  end
end

-- Argument validator: choice map (argument value maps to something)
builder._choice_map_validator = function( v, allowed )
  for k, value in pairs( allowed ) do
    if v:upper() == k:upper() then return value end
  end
end

-- Argument validator: string value (no validation)
builder._string_validator = function( v )
  return v
end

-- Argument printer: boolean value
builder._bool_printer = function( o )
  return "true|false", o.default and "true" or "false"
end

-- Argument printer: choice value
builder._choice_printer = function( o )
  local clist, opts  = '', o.data
  for i = 1, #opts do
    clist = clist .. ( i ~= 1 and "|" or "" ) .. opts[ i ]
  end
  return clist, o.default
end

-- Argument printer: choice map printer
builder._choice_map_printer = function( o )
  local clist, opts, def = '', o.data
  local i = 1
  for k, v in pairs( opts ) do
    clist = clist .. ( i ~= 1 and "|" or "" ) .. k
    if o.default == v then def = k end
    i = i + 1
  end
  return clist, def
end

-- Argument printer: string printer
builder._string_printer = function( o )
  return nil, o.default
end

-- Add an option of the specified type
builder._add_option = function( self, optname, opttype, help, default, data )
  local validators = 
  { 
    string = builder._string_validator, choice = builder._choice_validator, 
    boolean = builder._bool_validator, choice_map = builder._choice_map_validator
  }
  local printers = 
  { 
    string = builder._string_printer, choice = builder._choice_printer, 
    boolean = builder._bool_printer, choice_map = builder._choice_map_printer
  }
  if not validators[ opttype ] then
    print( sf( "[builder] Invalid option type '%s'", opttype ) )
    os.exit( 1 )
  end
  table.insert( self.options, { name = optname, help = help, validator = validators[ opttype ], printer = printers[ opttype ], data = data, default = default } )
end

-- Find an option with the given name
builder._find_option = function( self, optname )
  for i = 1, #self.options do
    local o = self.options[ i ]
    if o.name:upper() == optname:upper() then return self.options[ i ] end
  end
end

-- Helper: create the build output directory
builder._create_outdir = function( self )
  if self.output_dir_created then return end
   -- Create builds directory if needed
  local mode = lfs.attributes( self.build_dir, "mode" )
  if not mode or mode ~= "directory" then
    if not utils.full_mkdir( self.build_dir ) then
      print( "[builder] Unable to create directory " .. self.build_dir )
      os.exit( 1 )
    end
  end
  self.output_dir_created = true
end

-- 'add option' helper (automatically detects option type)
builder.add_option = function( self, name, help, default, data )
  local otype
  if type( default ) == 'boolean' then
    otype = 'boolean'
  elseif data and type( data ) == 'table' and #data == 0 then
    otype = 'choice_map'
  elseif data and type( data ) == 'table' then
    otype = 'choice'
    data = utils.linearize_array( data )
  elseif type( default ) == 'string' then
    otype = 'string'
  else
    print( sf( "[builder] Cannot detect option type for '%s'", name ) )
    os.exit( 1 )
  end
  self:_add_option( name, otype, help, default, data )
end

-- Initialize builder from the given command line
builder.init = function( self, args )
  -- Add the default options
  self:add_option( "build_mode", 'choose location of the object files', self.KEEP_DIR,
                   { keep_dir = self.KEEP_DIR, build_dir = self.BUILD_DIR, build_dir_linearized = self.BUILD_DIR_LINEARIZED } )
  self:add_option( "build_dir", 'choose build directory', self.build_dir )
  -- Apply default values to all options
  for i = 1, #self.options do
    local o = self.options[ i ]
    self.args[ o.name:upper() ] = o.default
  end
  -- Read and interpret command line
  for i = 1, #args do
    local a = args[ i ]
    if a:upper() == "-C" then                   -- clean option (-c)
      self.clean_mode = true  
    elseif a:upper() == '-H' then               -- help option (-h)
      self:_show_help()
      os.exit( 1 )
    elseif a:find( '=' ) then                   -- builder argument (key=value)
      local si, ei, k, v = a:find( "([^=]+)=(.*)$" )
      local opt = self:_find_option( k )
      if not opt then
        print( sf( "[builder] Invalid option '%s'", k ) )
        self:_show_help()
        os.exit( 1 )
      end
      local optv = opt.validator( v, opt.data )
      if optv == nil then
        print( sf( "[builder] Invalid value '%s' for option '%s'", v, k ) )
        self:_show_help()
        os.exit( 1 )
      end
      self.args[ k:upper() ] = optv
    else                                        -- this must be the target name / target arguments
      if self.targetname == nil then            
        self.targetname = a
      else
        table.insert( self.targetargs, a )
      end
    end
  end
  -- Read back the default options
  self.build_mode = self.args.BUILD_MODE
  self.build_dir = self.args.BUILD_DIR
end

-- Return the value of the option with the given name
builder.get_option = function( self, optname )
  return self.args[ optname:upper() ]
end

-- Show builder help
builder._show_help = function( self )
  print( "[builder] Valid options:" )
  print( "  -h: help (this text)" )
  print( "  -c: clean target" )
  for i = 1, #self.options do
    local o = self.options[ i ]
    print( sf( "\n  %s: %s", o.name, o.help ) )
    local values, default = o.printer( o )
    if values then
      print( sf( "    Possible values: %s", values ) )
    end
    print( sf( "    Default value: %s", default or "none (changes at runtime)" ) )
  end
end

---------------------------------------
-- Builder configuration

-- Set the compile command
builder.set_compile_cmd = function( self, cmd )
  self.comp_cmd = cmd
end

-- Set the link command
builder.set_link_cmd = function( self, cmd )
  self.link_cmd = cmd
end

-- Set the assembler command
builder.set_asm_cmd = function( self, cmd )
  self._asm_cmd = cmd
end

-- Set (actually force) the object file extension
builder.set_object_extension = function( self, ext )
  self.obj_extension = ext
end

-- Set (actually force) the executable file extension
builder.set_exe_extension = function( self, ext )
  self.exe_extension = ext
end

-- Set the clean mode
builder.set_clean_mode = function( self, isclean )
  self.clean_mode = isclean
end

-- Sets the build mode
builder.set_build_mode = function( self, mode )
  self.build_mode = mode
end

-- Set the output directory
builder.set_output_dir = function( self, dir )
  if self.output_dir_created then
    print "[ builder] Error: output directory already created"
    os.exit( 1 )
  end
  self.build_dir = dir
  self:_create_outdir()
end

-- Return the target arguments
builder.get_target_args = function( self )
  return self.targetargs
end

-- Set a specific dependency generation command for the assembler
-- Pass 'false' to skip dependency generation for assembler files
builder.set_asm_dep_cmd = function( self, asm_dep_cmd ) 
  self.asm_dep_cmd = asm_dep_cmd
end

-- Set a specific dependency generation command for the compiler
-- Pass 'false' to skip dependency generation for C files
builder.set_c_dep_cmd = function( self, c_dep_cmd )
  self.c_dep_cmd = c_dep_cmd
end

-- Save the builder configuration for a given component to a string
builder._config_to_string = function( self, what )
  local ctable = {}
  local state_fields 
  if what == 'comp' then
    state_fields = { 'comp_cmd', '_asm_cmd', 'c_dep_cmd', 'asm_dep_cmd', 'obj_extension' }
  elseif what == 'link' then
    state_fields = { 'link_cmd' }
  else
    print( sf( "Invalid argument '%s' to _config_to_string", what ) )
    os.exit( 1 )
  end
  utils.foreach( state_fields, function( k, v ) ctable[ v ] = self[ v ] end )
  return table.tostring( ctable )
end

-- Check the configuration of the given component against the previous one
-- Return true if the configuration has changed
builder._compare_config = function( self, what )
  local res = false
  local crtstate = self:_config_to_string( what )
  if not self.clean_mode then
    local fconf = io.open( self.build_dir .. dir_sep .. ".builddata." .. what, "rb" )
    if fconf then
      local oldstate = fconf:read( "*a" )
      fconf:close()
      if oldstate:lower() ~= crtstate:lower() then res = true end
    end
  end
  -- Write state to build dir
  fconf = io.open( self.build_dir .. dir_sep .. ".builddata." .. what, "wb" )
  if fconf then
    fconf:write( self:_config_to_string( what ) )
    fconf:close()
  end
  return res
end

---------------------------------------
-- Command line builders

-- Internal helper
builder._generic_cmd = function( self, args )
  local compcmd = args.compiler or "gcc"
  compcmd = compcmd .. " "
  local flags = type( args.flags ) == 'table' and table_to_string( utils.linearize_array( args.flags ) ) or args.flags
  local defines = type( args.defines ) == 'table' and table_to_string( utils.linearize_array( args.defines ) ) or args.defines
  local includes = type( args.includes ) == 'table' and table_to_string( utils.linearize_array( args.includes ) ) or args.includes
  local comptype = table_to_string( args.comptype ) or "-c"
  compcmd = compcmd .. utils.prepend_string( defines, "-D" )
  compcmd = compcmd .. utils.prepend_string( includes, "-I" )
  return compcmd .. flags .. " " .. comptype .. " -o $(TARGET) $(FIRST)"
end

-- Return a compile command based on the specified args
builder.compile_cmd = function( self, args )
  return self:_generic_cmd( args )
end

-- Return an assembler command based on the specified args
builder.asm_cmd = function( self, args )
  args.compiler = args.assembler
  return self:_generic_cmd( args )
end

-- Return a link command based on the specified args
builder.link_cmd = function( self, args )
  local flags = type( args.flags ) == 'table' and table_to_string( utils.linearize_array( args.flags ) ) or args.flags
  local libraries = type( args.libraries ) == 'table' and table_to_string( utils.linearize_array( args.libraries ) ) or args.libraries
  local linkcmd = args.linker or "gcc"
  linkcmd = linkcmd .. " " .. flags .. " -o $(TARGET) $(DEPENDS)"
  linkcmd = linkcmd .. " " .. utils.prepend_string( libraries, "-l" )
  return linkcmd
end

---------------------------------------
-- Target handling

-- Create a return a new C to object target
builder.c_target = function( self, target, deps, comp_cmd )
  return _target.new( target, deps, comp_cmd or self.comp_cmd, self )
end

-- Create a return a new ASM to object target
builder.asm_target = function( self, target, deps, asm_cmd )
  return _target.new( target, deps, asm_cmd or self._asm_cmd, self )
end

-- Return the name of a dependency file name corresponding to a C source
builder.get_dep_filename = function( self, srcname )
  return utils.replace_extension( self.build_dir .. dir_sep .. linearize_fname( srcname ), "d" )
end

-- Create a return a new C dependency target
builder.dep_target = function( self, dep, depdeps, dep_cmd )
  local depname = self:get_dep_filename( dep )
  return _target.new( depname, depdeps, dep_cmd, self )
end

-- Create and return a new link target
builder.link_target = function( self, out, dep, link_cmd )
  if not out:find( "%." ) and self.exe_extension and #self.exe_extension > 0 then
    out = out .. self.exe_extension
  end
  local t = _target.new( out, dep, link_cmd or self.link_cmd, self )
  if self:_compare_config( 'link' ) then t:force_rebuild( true ) end
  return t
end

-- Create and return a new generic target
builder.target = function( self, dest_target, deps, cmd )
  return _target.new( dest_target, deps, cmd, self )
end

-- Return the object name corresponding to a source file name
builder.obj_name = function( self, name )
  local r = self.obj_extension
  if not r then
    r = utils.is_windows() and "obj" or "o"
  end
  local objname = utils.replace_extension( name, r )
  -- KEEP_DIR: object file in the same directory as source file
  -- BUILD_DIR: object file in the build directory
  -- BUILD_DIR_LINEARIZED: object file in the build directory, linearized filename
  if self.build_mode == self.KEEP_DIR then 
    return objname
  elseif self.build_mode == self.BUILD_DIR_LINEARIZED then
    return self.build_dir .. dir_sep .. linearize_fname( objname )
  else
    local si, ei, path, fname = objname:find( "(.+)/(.-)$" )
    if not si then fname = objname end
    return self.build_dir .. dir_sep .. fname 
  end
end

-- Read and interpret dependencies for each file specified in "ftable"
-- "ftable" is either a space-separated string with all the source files or an array
builder.read_depends = function( self, ftable )
  if type( ftable ) == 'string' then ftable = utils.string_to_table( ftable ) end
  -- Read dependency data
  local dtable = {}
  for i = 1, #ftable do
    local f = io.open( self:get_dep_filename( ftable[ i ] ), "rb" )
    local lines = ftable[ i ]
    if f then
      lines = f:read( "*a" )
      f:close()
      lines = lines:gsub( "\n", " " ):gsub( "\\%s+", " " ):gsub( "%s+", " " ):gsub( "^.-: (.*)", "%1" )
    end
    dtable[ ftable[ i ] ] = lines
  end
  return dtable
end

-- Build and interpret dependencies for the given source files
-- "flable" is either a space-separated string with all the source files or an array
builder.make_depends = function( self, ftable )
  if type( ftable ) == 'string' then ftable = utils.string_to_table( ftable ) end

  -- Start with initial dependency data (this might be nil when generated initially)
  local initdep = self:read_depends( ftable )

  -- Build dependencies for all targets
  self.dtable = {}
  for i = 1, #ftable do
    local isasm = ftable[ i ]:find( "%.c$" ) == nil
    -- Skip assembler targets if 'asm_dep_cmd' is set to 'false'
    -- Skip C targets if 'c_dep_cmd' is set to 'false'
    local skip = isasm and self.asm_dep_cmd == false
    skip = skip or ( not isasm and self.c_dep_cmd == false )
    if not skip then
      local cmd = isasm and self._asm_cmd or self.comp_cmd
      local depcmd = cmd:gsub( "-c ", "-E -MM " )
      if isasm and self.asm_dep_cmd then depcmd = self.asm_dep_cmd end
      if not isasm and self.c_dep_cmd then depcmd = self.c_dep_cmd end
      local target = self:dep_target( ftable[ i ], initdep[ ftable[ i ] ], depcmd )
      -- The post build function will read the generated dependencies and save
      -- them into an instance-related field (dtable)
      target:set_post_build_function( function( t, _ ) 
        if not self.clean_mode then
          local tname = t.dep[ 1 ]:target_name()
          if tname then
            local fres = self:read_depends( tname )
            self.dtable[ tname ] = fres[ tname ] 
          end
        end
      end )
    end
  end
end

-- Create and return compile targets for the given sources
builder.create_compile_targets = function( self, ftable, res )
  if type( ftable ) == 'string' then ftable = utils.string_to_table( ftable ) end
  res = res or {}
  -- Build dependencies for all targets
  for i = 1, #ftable do
    local target 
    local deps = self:get_dep_filename( ftable[ i ] )
    if ftable[ i ]:find( "%.c$" ) then
      target = self:c_target( self:obj_name( ftable[ i ] ), { self.tlist[ deps ] or ftable[ i ] } )
    else
      target = self:asm_target( self:obj_name( ftable[ i ] ), { self.tlist[ deps ] or ftable[ i ] } )
    end
    -- Post build step: replace dependencies with the ones generated by 'make_depends'
    target:set_pre_build_function( function( t, _ ) 
      if not self.clean_mode then
        t:set_dependencies( self.dtable[ ftable[ i ] ] or ftable[ i ] )
      end
    end )
    table.insert( res, target )
  end
  return res
end

-- Add a target to the list of builder targets
builder.add_target = function( self, target, help, alias )
  self.targets[ target.target ] = { target = target, help = help }
  alias = alias or {}
  for _, v in ipairs( alias ) do
    self.targets[ v ] = { target = target, help = help }
  end
  return target
end

-- Make a target the default one
builder.default = function( self, target )
  self.deftarget = target.target
  self.targets.default = { target = target, help = "default target" }
end

-- Build everything
builder.build = function( self, target )
  local t = self.targetname or self.deftarget
  if not t then
    print( "[builder] Error: build target not specified" )
    os.exit( 1 )
  end
  if not self.targets[ t ] then
    print( sf( "[builder] Error: target '%s' not found", t ) )
    print( "Available targets: " )
    for k, v in pairs( self.targets ) do
      if not is_phony( k ) then 
        print( sf( "  %s - %s", k, v.help or "(no help available)" ) )
      end
    end
    if self.deftarget and not is_phony( self.deftarget ) then
      print( sf( "Default target is '%s'", self.deftarget ) )
    end
    os.exit( 1 )
  end
  self:_create_outdir()
  -- At this point check if we have a change in the state that would require a rebuild
  if self:_compare_config( 'comp' ) then
    print "[builder] Forcing rebuild due to configuration change"
    self.global_force_rebuild = true
  else
    self.global_force_rebuild = false
  end
  -- Do the actual build
  local res = self.targets[ t ].target:build()
  if not res then print( sf( '[builder] %s: up to date', t ) ) end
  if self.clean_mode then 
    os.remove( self.build_dir .. dir_sep .. ".builddata.comp" ) 
    os.remove( self.build_dir .. dir_sep .. ".builddata.link" ) 
  end
  print "[builder] Done building target."
  return res
end

-- Create dependencies, create object files, link final object
builder.make_exe_target = function( self, target, file_list )
  self:make_depends( file_list )
  local odeps = self:create_compile_targets( file_list )
  local exetarget = self:link_target( target, odeps )
  self:default( self:add_target( exetarget ) )
  return exetarget
end

-------------------------------------------------------------------------------
-- Other exported functions

function new_builder( build_dir )
  return builder.new( build_dir )
end

