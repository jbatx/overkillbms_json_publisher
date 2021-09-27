#!/bin/bash
zip 
aws lambda create-function --function-name  CreateTableAddRecordsAndRead --runtime python3.8 \
--zip-file fileb://app.zip --handler app.handler \
--role arn:aws:iam::123456789012:role/lambda-vpc-role \
--vpc-config SubnetIds=subnet-0532bb6758ce7c71f,subnet-d6b7fda068036e11f,SecurityGroupIds=sg-0897d5f549934c2fb
