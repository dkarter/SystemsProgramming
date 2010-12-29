#! /usr/bin/env ruby

require 'socket'
require 'openssl'

PUB_KEY = <<KEY
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApQYCG7WtKKCZ1KgtRhYr
rqG+1iruP2RFkanKA4TX6aIu/HSZVHlg64ntsHMayhuDsRfdUEFZc+e61Jd01b0/
R5UWEJPOJ4PdE7Vllwv3NqbclUBu6Q+Q+lTpeYBu860eP97u5Uk6NG4eVzlkzDYT
kur84qT5sCWjgd1vjY2yuY5u+3nMtx5CfZxl/MrIbxq5E+SG6a7LGBi1B9vWOTSl
ASvDBn3cRWZEcMyxorKvizAv/uWIoSiQXNqn5PUI6FgO5lBSbAHydt5ztkBBAfIC
PwQeDXlBzTshML0Kbl2FYRhGAdaFbOJwIwEMGNJlKNt+2ujvzJfn7/UW/bgTbDNT
OQIDAQAB
-----END PUBLIC KEY-----
KEY

class IO
  def write_long(n)
    write([n].pack('N'))
  end
end

class Handin
  HANDIN_HOST = 'ada.cs.iit.edu'
  HANDIN_PORT = 8088
  CIPHER_ALG  = 'aes-256-cbc'
  
  def initialize(course_id, lab_id, *files)
    @course_id = course_id
    @lab_id = lab_id
    @files = []
    files.each do |file|
      if File.exists?(file) && File.readable?(file)
        @files << file
      else
        raise "File #{file} does not exist!"
      end
    end
    @public_key = OpenSSL::PKey::RSA.new(PUB_KEY)
    @cipher = OpenSSL::Cipher::Cipher.new(CIPHER_ALG)
    @cipher.encrypt
    @cipher.key = @random_key = @cipher.random_key
    @cipher.iv  = @random_iv  = @cipher.random_iv
  end
  
  def <<(file)
    if File.exists?(file) && File.readable?(file)
      @files << file
    else
      raise "File #{file} does not exist!"
    end
  end
  
  def send(host=HANDIN_HOST, port=HANDIN_PORT)
    if @files.length == 0
      raise "No files to be submitted!"
    else
      sock = TCPSocket.open(host, port)
      enc_key = @public_key.public_encrypt(@random_key)
      enc_iv  = @public_key.public_encrypt(@random_iv)
      enc_data = @cipher.update(File.read("| tar c #{@files.join(' ')}"))
      enc_data << @cipher.final    
      
      sock.puts @course_id
      sock.puts @lab_id
      sock.write_long enc_key.length
      sock.write enc_key
      sock.write_long enc_iv.length
      sock.write enc_iv
      sock.write_long enc_data.length
      sock.write enc_data
      
      puts sock.gets # Get and print server response
      sock.close
    end
  rescue Errno::ECONNREFUSED, Errno::ECONNABORTED
    puts "The submission server is either down or is not accepting connections."
    puts "Submission unsuccessful"
  rescue => e
    puts "Submission unsuccessful, #{e.message}"
  end
end

if $0 == __FILE__
  Handin.new(*ARGV).send
end
