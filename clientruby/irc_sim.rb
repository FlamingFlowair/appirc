#!/usr/bin/ruby
require 'socket'
require 'gserver'


class Command
    attr_reader :code, :cde, :comment
  def initialize(code, cde, comment)
    @code=code
    @cde=cde
    @comment=comment
  end
end

class CommandList
  def initialize
    @commandsByName={}
    @commandsByCode={}
  end

  def load(filename)
    pattern=Regexp.new('^([0-9]+) +(\w+) +(.*)')
    File.open(filename) do |file|
      file.each_line { |line|
        if line !~ /^ *#/ && (m=pattern.match(line)) then
          cde=Command.new(m[1].to_i,m[2],m[3])
          @commandsByName[m[2].downcase]=cde
          @commandsByCode[m[1].to_i]=cde
        end
      }
    end
  end

  def code(cde)
    cde=cde.downcase
    raise "No command with name #{cde}" unless @commandsByName.key?(cde)
    return @commandsByName[cde].code
  end

  def cde(code)
    raise "No command with code #{code}" unless @commandsByCode.key?(code)
    return @commandsByCode[code].cde
  end

  def help(cde)
    cde=cde.downcase
    return @commandsByName[cde].comment
  end
end



class IrcThing
  def initialize()
    @serial=0
    @commands=CommandList.new
    @debug=true
  end

  def load(filename)
    @commands.load(filename)
  end

  def codeFrame(code, *args)
    args.flatten!
    data=args.join("\n")+"\n"
    frame=[data.bytesize()+3, @serial, code].pack("vvC")+data
    puts "S> #{@serial} - #{@commands.cde(code)}(#{code}) - #{args} <S" if @debug
    return frame
  end

  def decodeFrame(frame)
    serial,code,data = frame.unpack('vCa*')
    data=data.split("\n")
    cde=@commands.cde(code)
    puts "R> #{serial} - #{cde}(#{code}) - #{data.to_s} <R" if @debug
    return serial,cde,data
  end

  def write(output,cde, *args)
    if cde.kind_of?(String) then
      cde=@commands.code(cde)
    end
    frame=codeFrame(cde,args)
    output.write(frame)
    incSerial
  end

  def read(input)
    buf=input.read(2)
    return nil unless buf
    size=buf.unpack('v')[0]
    buf=input.read(size)
    return nil unless buf
    return decodeFrame(buf)
  end

  def incSerial()
    if @serial == 65535
      @serial=0
    else
      @serial+=1
    end
  end
end

class IrcClient < IrcThing
  def initialize(host,port)
    super()

    port=port.to_i
    @input=@output=TCPSocket.new(host,port)
    raise "Can't connect to #{host}:#{port}" unless @input
    puts "Connexion reussie" if @debug
  end

  def run(*args)
    # Un thread pour afficher ce qu'on recoit du serveur
    th=Thread.new(self) { |server| server.display_incoming() }

    # Pour la lecture: soit au clavier, soit on joue en séquence des scenarios
    if ( args.size() == 0 ) then # Mode intéractif
      get_input(STDIN)
      STDIN.close()
    else # Mode chargement de scénarios
      args.each { |arg| 
        File.open(arg) { |file| get_input(file) }
      }
    end
    th.join()
  end

  def display_incoming()
    while (frame=read(@input)) != nil  do
      serial,cde,data=frame
    end
    puts "**No more messages from server, connexion closed**"
  end

  # Lecture des commandes en mode texte envoyées par l'utilisateur
  # Format: texte séparé par des virgules
  def get_input(io)
    io.each_line { |line|
      begin
        line.chomp!()
        args=line.split(',')
        unless ( args.empty? ) then 
          args.each { |arg| arg.strip! }
          write(@output,*args)
        end
      rescue RuntimeError => erreur
        STDERR.puts erreur
      end
    }
  end
end

# Manque de l'héritage multiple en ruby
class IrcPongServer < GServer 
  def initialize(commandFile,port)
    puts "**Starting Pong Irc Server on port #{port.to_i}**"
    super(port.to_i)
    @commandFile=commandFile
  end

  def serve(client)
    irc=IrcThing.new()
    irc.load(@commandFile)
    puts "**Nouveau client #{client.to_s}**"
    while (frame=irc.read(client)) != nil  do 
      serial,commande,data=frame
      irc.write(client,commande, *data)
    end
    puts "** Client #{client.to_s} deconnecte**"
  end
end

class IrcSolo < IrcClient
  def initialize()
    @input,@output=IO.pipe()
  end
end

if ( ARGV[0] == '--server' ) then
  s=IrcPongServer.new('commandes.codes', ARGV[1].to_i) 
  s.audit
  s.start
  s.join
elsif ( ARGV.empty? || ARGV[0] == '-h' || ARGV[0] == '--help' )
  STDERR.puts("Usage: irc_sim.rb serverHost port
Tapez ensuite des commandes au format:
commande[, arg1, arg2...]
Les commandes seront envoyee au serveur en respectant le protocole. Les commandes envoyees sont affichees comme suit:

   S> 0 - join(21) - [\"iut\"] <S
      0: numero de sequence (sera incremente automatiquement)
      join: commande, 21: code de la commande dans le protocole
      \"iut\": unique argument

Les commandes recues sont affichees de la meme facon.
Les commandes sont donnes dans le fichier 'commandes.code'")

else
  ARGV.shift if ARGV[0] == "--client"
  s=IrcClient.new(ARGV[0],ARGV[1].to_i) 
  s.load('commandes.codes')
  s.run()
end
