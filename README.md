# mf-embedded
OCF based application that allows for a modular system, where a user will be able to set up their own devices by adding premade modules

To set the number of components: `idf.py build -DMF_NUMBER_COMPONENTS=<number>`

# Instructions
1. docker run -v $HOME/plgd/data:/data -d --rm --name mf_cloud -p 443:443 -p 5683:5683 -p 5684:5684 mf_cloud:latest
2.
