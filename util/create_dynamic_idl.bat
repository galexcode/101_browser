wget http://www.whatwg.org/specs/web-apps/current-work/ -O html5.html
java -cp htmlparser-1.2.1.jar nu.validator.htmlparser.tools.XSLT4HTML5 --template=create_html5_idl.xslt --input-html=html5.html --output-xml=../data/idl/temp_.idl
sed -e '1d' -e "s/&lt;/</" -e "s/&gt;/>/" ../data/idl/temp_.idl > ../data/idl/html5.idl


wget http://www.whatwg.org/specs/web-workers/current-work/ -O webworkers.html
java -cp htmlparser-1.2.1.jar nu.validator.htmlparser.tools.XSLT4HTML5 --template=create_webworkers_idl.xslt --input-html=webworkers.html --output-xml=../data/idl/temp_.idl
sed -e '1d' -e "s/&lt;/</" -e "s/&gt;/>/" ../data/idl/temp_.idl > ../data/idl/webworkers.idl


rm ../data/idl/temp_.idl
rm html5.html
rm webworkers.html
